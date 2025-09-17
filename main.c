#include <furi.h>
#include "main.h"
#include <gui/gui.h>
#include <string.h>
#include <stdlib.h>

void usb_hid_paste(void* context, char* str) {
    AppContext* app = context;
    furi_hal_usb_enable();
    furi_hal_usb_unlock();

    furi_check(furi_hal_usb_set_config(&usb_hid, NULL));
    furi_delay_ms(300);
    for(size_t i = 0; i < strlen(str); i++) {
        furi_hal_hid_kb_press(USB_ASCII_TO_KEY(app, str[i]));
        furi_hal_hid_kb_release(USB_ASCII_TO_KEY(app, str[i]));
        furi_delay_ms(15);
    }
    furi_hal_usb_set_config(&usb_cdc_single, NULL);
    furi_hal_usb_lock();
}

void ble_hid_paste(void* context, char* str) {
    AppContext* app = context;

    furi_delay_ms(300);

    for(size_t i = 0; i < strlen(str); i++) {
        ble_profile_hid_kb_press(app->ble, USB_ASCII_TO_KEY(app, str[i]));
        ble_profile_hid_kb_release(app->ble, USB_ASCII_TO_KEY(app, str[i]));

        furi_delay_ms(15);
    }
}

void password_manager_enhanced_add_to_queue_callback(const void* input, void* context) {
    const InputEvent* event = input;
    FuriMessageQueue* queue = context;
    furi_message_queue_put(queue, event, 0);
}

void password_manager_enhanced_handle_csv(void* context) {
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
            FuriPubSubSubscription* subscription = furi_pubsub_subscribe(
                input, password_manager_enhanced_add_to_queue_callback, queue);
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

bool password_manager_enhanced_input_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    AppContext* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool password_manager_enhanced_input_back_event_callback(void* context) {
    furi_assert(context);
    AppContext* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

AppContext* app_alloc() {
    AppContext* app = malloc(sizeof(AppContext));

    app->bt = furi_record_open(RECORD_BT);
    app->scene_manager = scene_manager_alloc(&password_manager_enhanced_input_scene_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, password_manager_enhanced_input_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, password_manager_enhanced_input_back_event_callback);
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
        app->view_dispatcher, AppView_password_dialog, dialog_ex_get_view(app->dialog));
    view_dispatcher_add_view(
        app->view_dispatcher, AppView_username_dialog, dialog_ex_get_view(app->dialog));
    app->options = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, AppView_options, variable_item_list_get_view(app->options));

    app->dialogs = furi_record_open(RECORD_DIALOGS);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_file_exists(storage, OPT_LOAD_PATH)) {
        File* file = storage_file_alloc(storage);
        if(storage_file_open(file, OPT_LOAD_PATH, FSAM_WRITE, FSOM_CREATE_NEW)) {
            char buff[64];
            strcpy(buff, "0\nen-US.kl");
            storage_file_write(file, buff, strlen(buff));

            app->using_ble = false;
        }
        storage_file_close(file);
        storage_file_free(file);
    } else {
        File* file = storage_file_alloc(storage);
        if(storage_file_open(file, OPT_LOAD_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
            char ch;
            storage_file_read(file, &ch, 1);
            if(ch == '0') {
                app->using_ble = false;
                storage_file_seek(file, 1, false);
            } else if(ch == '1') {
                app->using_ble = true;
                storage_file_seek(file, 1, false);
            }
            char buff[64] = {0};
            size_t buff_pos = 0;
            while(storage_file_read(file, &ch, 1)) {
                buff[buff_pos++] = ch;
            }
            storage_file_close(file);
            storage_file_free(file);
            buff[buff_pos] = '\0';
            File* kb_file = storage_file_alloc(storage);
            char kb_path[sizeof(buff) + 64];

            snprintf(kb_path, sizeof(kb_path), "%s%s", KBL_LOAD_PATH, buff);
            if(storage_file_open(kb_file, kb_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
                uint16_t layout[128];
                if(storage_file_read(kb_file, layout, sizeof(layout)) == sizeof(layout)) {
                    memcpy(app->kbl, layout, sizeof(layout));
                }
            }
            storage_file_close(kb_file);
            storage_file_free(kb_file);
        }
    }
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
    view_dispatcher_remove_view(app->view_dispatcher, AppView_password_dialog);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_username_dialog);

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
