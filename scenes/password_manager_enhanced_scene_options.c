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
            furi_string_set(app->file_path, browser_options.base_path);
            if(dialog_file_browser_show(
                   app->dialogs, app->file_path, app->file_path, &browser_options)) {
                scene_manager_next_scene(app->scene_manager, AppScene_options);
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
