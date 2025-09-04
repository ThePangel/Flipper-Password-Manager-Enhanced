#include <furi.h>
#include "main.h"
#include <gui/gui.h>
#include <string.h>
#include <stdlib.h>
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>

#include "flipper_password_manager_enhanced_icons.h"

#define CSV_LOAD_PATH EXT_PATH("apps_data") "/password_manager_enhanced"

static void add_to_queue_callback(const void* input, void* context) {
    const InputEvent* event = input;
    FuriMessageQueue* queue = context;
    furi_message_queue_put(queue, event, 0);
}

void app_handle_csv(void* context) {
    AppContext* app = context;
    if(!((app->file_name == app->file_path) && app->entry_count > 0)) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(storage);
        size_t count = 0;
        CsvEntry entry = {0};
        int col = 0;

        if(storage_file_open(
               file, furi_string_get_cstr(app->file_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            char ch;

            char line[512] = {0};
            size_t line_pos = 0;
            bool skip_headers = true;
            FuriMessageQueue* queue = furi_message_queue_alloc(1, sizeof(InputEvent));
            FuriPubSub* input = furi_record_open(RECORD_INPUT_EVENTS);
            FuriPubSubSubscription* subscription =
                furi_pubsub_subscribe(input, add_to_queue_callback, queue);
            InputEvent event;
            event.key = InputKeyOk;
            furi_message_queue_put(queue, &event, 0);

            app->entry = NULL;

            while(storage_file_read(file, &ch, 1) && !storage_file_eof(file)) {
                if(furi_message_queue_get(queue, &event, 0)) {
                    if(event.key == InputKeyBack) {
                        furi_message_queue_free(queue);
                        furi_pubsub_unsubscribe(input, subscription);
                        furi_record_close(RECORD_INPUT_EVENTS);
                        storage_file_close(file);
                        storage_file_free(file);
                        furi_record_close(RECORD_STORAGE);
                        return;
                    }
                }

                if(line_pos < sizeof(line) - 1) {
                    line[line_pos++] = ch;
                    line[line_pos] = '\0';
                }

                if(ch == '\n' || ch == ',') {
                    if(line_pos > 0) {
                        line[line_pos - 1] = '\0';
                    }

                    if(!skip_headers) {
                        switch(col) {
                        case 0:
                            entry.name = malloc(strlen(line) + 1);
                            strcpy(entry.name, line);
                            break;
                        case 2:
                            entry.username = malloc(strlen(line) + 1);
                            strcpy(entry.username, line);
                            break;
                        case 3:
                            entry.password = malloc(strlen(line) + 1);
                            strcpy(entry.password, line);
                            break;
                        }
                    }
                    col++;

                    if(count % 20 == 0) {
                        widget_reset(app->loading);
                        char text[64];
                        snprintf(text, sizeof(text), "CSV parsing: %u", count);
                        widget_add_string_element(
                            app->loading, 64, 32, AlignCenter, AlignCenter, FontPrimary, text);
                    }
                    line_pos = 0;

                    if(ch == '\n') {
                        if(skip_headers) {
                            skip_headers = false;
                            col = 0;
                        } else {
                            CsvEntry* new_entry =
                                realloc(app->entry, (count + 1) * sizeof(CsvEntry));

                            app->entry = new_entry;
                            app->entry[count] = entry;

                            count++;

                            col = 0;
                            memset(&entry, 0, sizeof(entry));
                        }
                    }
                }
            }

            furi_message_queue_free(queue);
            furi_pubsub_unsubscribe(input, subscription);
            furi_record_close(RECORD_INPUT_EVENTS);
        }

        app->entry_count = count;
        app->file_name = app->file_path;
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
    }

    scene_manager_next_scene(app->scene_manager, AppScene_Submenu);
}
bool app_scene_on_event_main_menu(void* context, SceneManagerEvent event) {
    AppContext* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case AppEvent_FileBrowser:
            scene_manager_next_scene(app->scene_manager, AppScene_Submenu);
            consumed = true;
            break;
        }
        break;
    case SceneManagerEventTypeBack:
        view_dispatcher_stop(app->view_dispatcher);
        consumed = true;
        break;
    default:
        break;
    }
    return consumed;
}

bool app_scene_on_event_sub_menu(void* context, SceneManagerEvent event) {
    AppContext* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case AppEvent_FileBrowser:
            scene_manager_next_scene(app->scene_manager, AppScene_Submenu);
            consumed = true;
            break;
        }
        break;
    case SceneManagerEventTypeBack:
        scene_manager_next_scene(app->scene_manager, AppScene_MainMenu);
        consumed = true;
        break;
    default:
        consumed = false;
        break;
    }
    return consumed;
}

bool app_scene_on_event_loading_screen(void* context, SceneManagerEvent event) {
    AppContext* app = context;
    UNUSED(app);
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeBack:
        scene_manager_next_scene(app->scene_manager, AppScene_MainMenu);
        consumed = true;
        break;

    default:
        consumed = false;
        break;
    }
    return consumed;
}

bool app_scene_on_event_password_menu(void* context, SceneManagerEvent event) {
    AppContext* app = context;
    UNUSED(app);
    bool consumed = false;
    switch(event.type) {
    default:
        consumed = false;
        break;
    }
    return consumed;
}

void app_menu_callback_main_menu(void* context, uint32_t index) {
    AppContext* app = context;
    switch(index) {
    case 0:

        DialogsFileBrowserOptions browser_options;
        dialog_file_browser_set_basic_options(&browser_options, ".csv", &I_u2f_10px);
        browser_options.hide_dot_files = true;
        browser_options.hide_ext = false;
        browser_options.base_path = CSV_LOAD_PATH;
        furi_string_set(app->file_path, browser_options.base_path);
        if(dialog_file_browser_show(
               app->dialogs, app->file_path, app->file_path, &browser_options)) {
            scene_manager_next_scene(app->scene_manager, AppScene_loading);
            app_handle_csv(app);
        }
        break;
    }
}

void app_dialog_username_callback(DialogExResult result, void* context) {
    AppContext* app = context;
    switch(result) {
    case DialogExResultLeft:
        view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Submenu);
        break;
    case DialogExResultCenter:
        furi_hal_usb_enable();
        furi_hal_usb_unlock();
        furi_check(furi_hal_usb_set_config(&usb_hid, NULL));
        furi_delay_ms(100);
        fuir_hal_ char* str = app->entry[app->entry_index].username;
        for(size_t i = 0; i < strlen(str); i++) {
            furi_hal_hid_kb_press(HID_ASCII_TO_KEY(str[i]));
            furi_hal_hid_kb_release(HID_ASCII_TO_KEY(str[i]));
            furi_delay_ms(10);
        }
    default:
        break;
    }
}

void app_menu_username_dialog_callback(void* context, uint32_t index) {
    AppContext* app = context;
    dialog_ex_reset(app->dialog);
    dialog_ex_set_header(app->dialog, "Username:", 64, 16, AlignCenter, AlignCenter);
    dialog_ex_set_text(app->dialog, app->entry[index].username, 64, 32, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(app->dialog, "Back");
    dialog_ex_set_center_button_text(app->dialog, "Paste");
    dialog_ex_set_right_button_text(app->dialog, "Paswd");
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_result_callback(app->dialog, app_dialog_username_callback);
    app->entry_index = index;
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_dialog);
}

void app_menu_setup_password_menu(void* context, uint32_t index) {
    AppContext* app = context;
    scene_manager_next_scene(app->scene_manager, AppScene_password_menu);
    submenu_reset(app->password_menu);
    submenu_set_header(app->password_menu, app->entry[index].name);
    char username[64];
    snprintf(username, sizeof(username), "Username: %s", app->entry[index].username);
    submenu_add_item(app->password_menu, username, index, app_menu_username_dialog_callback, app);
    submenu_add_item(app->password_menu, "Reveal password", index + 1, NULL, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_password_menu);
}

void app_scene_on_enter_main_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->menu);
    submenu_add_item(app->menu, "View passwords", 0, app_menu_callback_main_menu, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_MainSMenu);
}
void app_scene_on_enter_sub_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->submenu);
    for(size_t i = 0; i < app->entry_count; i++) {
        submenu_add_item(app->submenu, app->entry[i].name, i, app_menu_setup_password_menu, app);
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Submenu);
}

void app_scene_on_enter_loading_screen(void* context) {
    AppContext* app = context;
    widget_reset(app->loading);
    widget_add_string_element(
        app->loading, 64, 32, AlignCenter, AlignCenter, FontPrimary, "CSV parsing: Counting");
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Loading);
}

void app_scene_on_enter_password_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->password_menu);
    scene_manager_handle_custom_event(app->scene_manager, AppEvent_LoadPasswrd);
}

void app_scene_on_exit_sub_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->submenu);
}

void app_scene_on_exit_main_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->menu);
}

void app_scene_on_exit_loading_screen(void* context) {
    AppContext* app = context;
    widget_reset(app->loading);
}

void app_scene_on_exit_password_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->password_menu);
}

void (*const app_scene_on_enter_handlers[])(void*) = {
    app_scene_on_enter_main_menu,
    app_scene_on_enter_sub_menu,
    app_scene_on_enter_loading_screen,
    app_scene_on_enter_password_menu,
};

bool (*const app_scene_on_event_handlers[])(void*, SceneManagerEvent) = {
    app_scene_on_event_main_menu,
    app_scene_on_event_sub_menu,
    app_scene_on_event_loading_screen,
    app_scene_on_event_password_menu,

};

void (*const app_scene_on_exit_handlers[])(void*) = {
    app_scene_on_exit_main_menu,
    app_scene_on_exit_sub_menu,
    app_scene_on_exit_loading_screen,
    app_scene_on_exit_password_menu,
};

const SceneManagerHandlers app_scene_event_handlers = {
    .on_enter_handlers = app_scene_on_enter_handlers,
    .on_event_handlers = app_scene_on_event_handlers,
    .on_exit_handlers = app_scene_on_exit_handlers,
    .scene_num = AppScene_count};

bool app_scene_custom_callback(void* context, uint32_t custom_event_id) {
    AppContext* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, custom_event_id);
}

bool app_back_event_callback(void* context) {
    AppContext* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

AppContext* app_alloc() {
    AppContext* app = malloc(sizeof(AppContext));

    app->scene_manager = scene_manager_alloc(&app_scene_event_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, app_scene_custom_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, app_back_event_callback);
    app->menu = submenu_alloc();
    view_dispatcher_add_view(app->view_dispatcher, AppView_MainSMenu, submenu_get_view(app->menu));
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, AppView_Submenu, submenu_get_view(app->submenu));
    app->loading = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, AppView_Loading, widget_get_view(app->loading));
    app->password_menu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, AppView_password_menu, submenu_get_view(app->password_menu));
    app->dialog = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, AppView_dialog, dialog_ex_get_view(app->dialog));

    app->dialogs = furi_record_open(RECORD_DIALOGS);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, CSV_LOAD_PATH);
    furi_record_close(RECORD_STORAGE);
    app->file_path = furi_string_alloc();
    app->file_name = furi_string_alloc();

    return app;
}

void app_free(void* context) {
    AppContext* app = context;

    scene_manager_free(app->scene_manager);

    view_dispatcher_remove_view(app->view_dispatcher, AppView_MainSMenu);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Submenu);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Loading);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_password_menu);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_dialog);

    view_dispatcher_free(app->view_dispatcher);

    submenu_free(app->menu);
    submenu_free(app->submenu);
    widget_free(app->loading);
    submenu_free(app->password_menu);
    dialog_ex_free(app->dialog);
    furi_record_close(RECORD_DIALOGS);
    furi_string_free(app->file_path);
    furi_string_free(app->file_name);
    if(app->entry) {
        for(size_t i = 0; i < app->entry_count; i++) {
            if(app->entry[i].name) free(app->entry[i].name);
            if(app->entry[i].username) free(app->entry[i].username);
            if(app->entry[i].password) free(app->entry[i].password);
        }
        free(app->entry);
    }
    free(app);
}

int32_t flipper_pass_manager_app(void* p) {
    UNUSED(p);

    AppContext* app = app_alloc();
    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(app->scene_manager, AppScene_MainMenu);
    view_dispatcher_run(app->view_dispatcher);

    app_free(app);
    furi_record_close(RECORD_GUI);
    return 0;
}
