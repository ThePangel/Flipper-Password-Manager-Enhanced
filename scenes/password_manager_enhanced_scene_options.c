#include "../main.h"

void password_manager_enhanced_scene_options_callback(void* context, uint32_t index) {
    AppContext* app = context;

    switch(index) {
    case 1:
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_FileBrowser);

    default:
        break;
    }
}

void password_manager_enhanced_scene_options_callback_conection(VariableItem* item) {
    AppContext* app = variable_item_get_context(item);
    if(app->using_ble) {
        variable_item_set_current_value_text(item, "USB");

        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_ChangeSettings);
    } else {
        variable_item_set_current_value_text(item, "BLE");

        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_ChangeSettings);
    }
}

void password_manager_enhanced_scene_options_on_enter(void* context) {
    AppContext* app = context;
    variable_item_list_reset(app->options);
    VariableItem* bluetooth = variable_item_list_add(
        app->options,
        "Connection",
        2,
        password_manager_enhanced_scene_options_callback_conection,
        app);
    VariableItem* set_kbl = variable_item_list_add(app->options, "Keyboard Layout", 1, NULL, app);
    if(app->using_ble) {
        variable_item_set_current_value_text(bluetooth, "BLE");
    } else {
        variable_item_set_current_value_text(bluetooth, "USB");
    }

    UNUSED(set_kbl);
    variable_item_list_set_enter_callback(
        app->options, password_manager_enhanced_scene_options_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_options);
}

bool password_manager_enhanced_scene_options_on_event(void* context, SceneManagerEvent event) {
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
            FuriString* path = furi_string_alloc();
            furi_string_set(path, browser_options.base_path);

            if(dialog_file_browser_show(app->dialogs, path, path, &browser_options)) {
                furi_string_right(
                    path, furi_string_search_rchar(path, '/', 0) + 1

                );
                Storage* storage = furi_record_open(RECORD_STORAGE);
                File* file = storage_file_alloc(storage);
                if(storage_file_open(file, OPT_LOAD_PATH, FSAM_WRITE, FSOM_OPEN_EXISTING)) {
                    const char* path_str = furi_string_get_cstr(path);
                    storage_file_seek(file, 2, true);
                    storage_file_write(file, path_str, sizeof(path_str));
                    //storage_file_write(file, "\n\0", 3);
                }
                storage_file_close(file);
                storage_file_free(file);

                File* kb_file = storage_file_alloc(storage);
                char kb_path[sizeof(furi_string_get_cstr(path)) + 64];
                snprintf(
                    kb_path, sizeof(kb_path), "%s%s", KBL_LOAD_PATH, furi_string_get_cstr(path));
                FURI_LOG_I("gang", kb_path);
                if(storage_file_open(kb_file, kb_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
                    uint16_t layout[128];
                    if(storage_file_read(kb_file, layout, sizeof(layout)) == sizeof(layout)) {
                        memcpy(app->kbl, layout, sizeof(layout));
                    }
                }
                storage_file_close(kb_file);
                storage_file_free(kb_file);
                furi_record_close(RECORD_STORAGE);
            }
            break;

        case AppEvent_ChangeSettings:
            Storage* storage = furi_record_open(RECORD_STORAGE);
            File* file = storage_file_alloc(storage);
            if(app->using_ble) {
                app->using_ble = false;
                if(storage_file_open(file, OPT_LOAD_PATH, FSAM_WRITE, FSOM_OPEN_EXISTING)) {
                    char ch = '0';

                    storage_file_write(file, &ch, 1);
                }

            } else {
                app->using_ble = true;
                if(storage_file_open(file, OPT_LOAD_PATH, FSAM_WRITE, FSOM_OPEN_EXISTING)) {
                    char ch = '1';

                    storage_file_write(file, &ch, 1);
                }
            }
            storage_file_close(file);
            storage_file_free(file);
            furi_record_close(RECORD_STORAGE);
            break;
        }
        break;
    default:
        consumed = false;
        break;
    }
    return consumed;
}

void password_manager_enhanced_scene_options_on_exit(void* context) {
    AppContext* app = context;
    variable_item_list_reset(app->options);
}
