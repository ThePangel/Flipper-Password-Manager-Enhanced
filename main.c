#include <furi.h>
#include "main.h"
#include <gui/gui.h>
#include <string.h>
#include <stdlib.h>

#include "password_manager_enhanced_icons.h"
#define USB_ASCII_TO_KEY(x) (((uint8_t)x < 128) ? (test[(uint8_t)x]) : HID_KEYBOARD_NONE)

#define CSV_LOAD_PATH APP_DATA_PATH("")
#define KBL_LOAD_PATH APP_ASSETS_PATH("")

static void usb_hid_paste(char* str) {
    furi_hal_usb_enable();
    furi_hal_usb_unlock();

    furi_check(furi_hal_usb_set_config(&usb_hid, NULL));
    furi_delay_ms(300);
    for(size_t i = 0; i < strlen(str); i++) {
        furi_hal_hid_kb_press(USB_ASCII_TO_KEY(str[i]));
        furi_hal_hid_kb_release(USB_ASCII_TO_KEY(str[i]));
        furi_delay_ms(15);
    }
}

static void ble_hid_paste(void* context, char* str) {
    AppContext* app = context;

    furi_delay_ms(300);

    for(size_t i = 0; i < strlen(str); i++) {
        ble_profile_hid_kb_press(app->ble, USB_ASCII_TO_KEY(str[i]));
        ble_profile_hid_kb_release(app->ble, USB_ASCII_TO_KEY(str[i]));

        furi_delay_ms(15);
    }
}

static void add_to_queue_callback(const void* input, void* context) {
    const InputEvent* event = input;
    FuriMessageQueue* queue = context;
    furi_message_queue_put(queue, event, 0);
}

void app_handle_csv(void* context) {
    AppContext* app = context;

    if(!(furi_string_cmp(app->file_name, app->file_path) == 0 && app->entry_count > 0)) {
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
        furi_string_set(app->file_name, app->file_path);
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
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case AppEvent_PastePasswrd:
            char* user = app->entry[app->entry_index].username;
            char* pass = app->entry[app->entry_index].password;
            if(app->using_ble) {
                ble_hid_paste(app, user);
                ble_profile_hid_kb_press(app->ble, HID_KEYBOARD_TAB);
                ble_profile_hid_kb_release(app->ble, HID_KEYBOARD_TAB);
                ble_hid_paste(app, pass);
            } else {
                usb_hid_paste(user);
                furi_hal_hid_kb_press(HID_KEYBOARD_TAB);
                furi_hal_hid_kb_release(HID_KEYBOARD_TAB);
                usb_hid_paste(pass);
            }
            consumed = true;
            break;
        default:
            break;
        }
        break;

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
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_FileBrowser);

        break;
    case 1:
        scene_manager_next_scene(app->scene_manager, AppScene_options);
        break;
    }
}

void app_dialog_username_callback(DialogExResult result, void* context) {
    AppContext* app = context;
    switch(result) {
    case DialogExResultLeft:
        view_dispatcher_switch_to_view(app->view_dispatcher, AppView_password_menu);
        break;
    case DialogExResultCenter:
        char* str = app->entry[app->entry_index].username;
        if(app->using_ble)
            ble_hid_paste(app, str);
        else
            usb_hid_paste(str);

        break;

    case DialogExResultRight:
        app_menu_password_dialog_callback(app, app->entry_index);
        break;
    default:
        break;
    }
}

void app_dialog_password_callback(DialogExResult result, void* context) {
    AppContext* app = context;
    switch(result) {
    case DialogExResultLeft:
        app_menu_username_dialog_callback(app, app->entry_index);
        break;
    case DialogExResultCenter:

        char* str = app->entry[app->entry_index].password;
        if(app->using_ble)
            ble_hid_paste(app, str);
        else
            usb_hid_paste(str);

        break;
    default:
        break;
    }
}
void app_menu_password_dialog_callback(void* context, uint32_t index) {
    AppContext* app = context;
    dialog_ex_reset(app->dialog);
    dialog_ex_set_header(app->dialog, "Password:", 64, 16, AlignCenter, AlignCenter);
    dialog_ex_set_text(app->dialog, app->entry[index].password, 64, 32, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(app->dialog, "Back");
    dialog_ex_set_center_button_text(app->dialog, "Paste");

    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_result_callback(app->dialog, app_dialog_password_callback);
    app->entry_index = index;
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_dialog);
}

void app_menu_username_dialog_callback(void* context, uint32_t index) {
    AppContext* app = context;
    dialog_ex_reset(app->dialog);
    dialog_ex_set_header(app->dialog, "Username:", 64, 16, AlignCenter, AlignCenter);
    dialog_ex_set_text(app->dialog, app->entry[index].username, 64, 32, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(app->dialog, "Back");
    dialog_ex_set_center_button_text(app->dialog, "Paste");
    dialog_ex_set_right_button_text(app->dialog, "Pasw");
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_result_callback(app->dialog, app_dialog_username_callback);
    app->entry_index = index;
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_dialog);
}

void app_menu_paste_callback(void* context, uint32_t index) {
    AppContext* app = context;
    app->entry_index = index - 2;
    view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_PastePasswrd);
}

void app_menu_setup_password_menu(void* context, uint32_t index) {
    AppContext* app = context;
    scene_manager_next_scene(app->scene_manager, AppScene_password_menu);
    submenu_reset(app->password_menu);
    submenu_set_header(app->password_menu, app->entry[index].name);
    char username[64];
    snprintf(username, sizeof(username), "Username: %s", app->entry[index].username);
    submenu_add_item(app->password_menu, username, index, app_menu_username_dialog_callback, app);
    submenu_add_item(
        app->password_menu, "Reveal password", index + 1, app_menu_password_dialog_callback, app);
    submenu_add_item(app->password_menu, "Paste", index + 2, app_menu_paste_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_password_menu);
}

void app_scene_on_enter_main_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->menu);
    submenu_add_item(app->menu, "View passwords", 0, app_menu_callback_main_menu, app);
    submenu_add_item(app->menu, "Options", 1, app_menu_callback_main_menu, app);
    submenu_add_item(app->menu, "About", 2, app_menu_callback_main_menu, app);
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

void app_options_callback(void* context, uint32_t index) {
    AppContext* app = context;

    switch(index) {
    case 1:
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_FileBrowser);

    default:
        break;
    }
}

void app_scene_on_enter_options(void* context) {
    AppContext* app = context;
    variable_item_list_reset(app->options);
    VariableItem* bluetooth = variable_item_list_add(app->options, "Bluetooth", 2, NULL, app);
    VariableItem* set_kbl = variable_item_list_add(app->options, "Keyboard Layout", 1, NULL, app);
    UNUSED(bluetooth);
    UNUSED(set_kbl);

    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_options);
}

bool app_scene_on_event_options(void* context, SceneManagerEvent event) {
    AppContext* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case AppEvent_FileBrowser:
            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(&browser_options, ".kl", &I_keyboard_10px);
            browser_options.hide_dot_files = true;
            browser_options.hide_ext = false;
            browser_options.base_path = KBL_LOAD_PATH;
            furi_string_set(app->file_path, browser_options.base_path);
            if(dialog_file_browser_show(
                   app->dialogs, app->file_path, app->file_path, &browser_options)) {
                scene_manager_next_scene(app->scene_manager, AppScene_options);
            }
            break;
        }
        break;

    default:
        consumed = false;
        break;
    }
    return consumed;
}

void app_scene_on_exit_options(void* context) {
    AppContext* app = context;
    variable_item_list_reset(app->options);
}

void (*const app_scene_on_enter_handlers[])(void*) = {
    app_scene_on_enter_main_menu,
    app_scene_on_enter_sub_menu,
    app_scene_on_enter_loading_screen,
    app_scene_on_enter_password_menu,
    app_scene_on_enter_options,
};

bool (*const app_scene_on_event_handlers[])(void*, SceneManagerEvent) = {
    app_scene_on_event_main_menu,
    app_scene_on_event_sub_menu,
    app_scene_on_event_loading_screen,
    app_scene_on_event_password_menu,
    app_scene_on_event_options,

};

void (*const app_scene_on_exit_handlers[])(void*) = {
    app_scene_on_exit_main_menu,
    app_scene_on_exit_sub_menu,
    app_scene_on_exit_loading_screen,
    app_scene_on_exit_password_menu,
    app_scene_on_exit_options,
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
    app->bt = furi_record_open(RECORD_BT);
    app->scene_manager = scene_manager_alloc(&app_scene_event_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();
    app->using_ble = false;
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
    app->options = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, AppView_options, variable_item_list_get_view(app->options));
    variable_item_list_set_enter_callback(app->options, app_options_callback, app);

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
    furi_hal_usb_set_config(&usb_cdc_single, NULL);
    furi_hal_usb_lock();

    scene_manager_free(app->scene_manager);

    view_dispatcher_remove_view(app->view_dispatcher, AppView_MainSMenu);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Submenu);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Loading);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_password_menu);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_dialog);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_options);

    view_dispatcher_free(app->view_dispatcher);

    submenu_free(app->menu);
    submenu_free(app->submenu);
    widget_free(app->loading);
    submenu_free(app->password_menu);
    dialog_ex_free(app->dialog);
    variable_item_list_free(app->options);
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
    furi_record_close(RECORD_BT);
    app->bt = NULL;
    free(app);
}

int32_t flipper_pass_manager_app(void* p) {
    UNUSED(p);

    AppContext* app = app_alloc();
    Gui* gui = furi_record_open(RECORD_GUI);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    storage_common_migrate(
        storage,
        EXT_PATH("apps/Bluetooth/" HID_BT_KEYS_STORAGE_NAME),
        APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));

    bt_disconnect(app->bt);

    furi_delay_ms(200);

    bt_keys_storage_set_storage_path(app->bt, APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));

    furi_record_close(RECORD_STORAGE);

    app->ble = bt_profile_start(app->bt, ble_profile_hid, NULL);
    if(!app->ble) {
        FURI_LOG_E("FPME", "Failed to start BLE profile");
    }

    furi_hal_bt_start_advertising();
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(app->scene_manager, AppScene_MainMenu);
    view_dispatcher_run(app->view_dispatcher);
    bt_disconnect(app->bt);

    furi_delay_ms(200);

    bt_keys_storage_set_default_path(app->bt);

    if(!bt_profile_restore_default(app->bt)) {
        FURI_LOG_E("FPME", "Failed to switch to Serial profile");
    }

    app_free(app);
    furi_record_close(RECORD_GUI);
    return 0;
}
