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

void password_manager_enhanced_scene_options_on_enter(void* context) {
    AppContext* app = context;
    variable_item_list_reset(app->options);
    VariableItem* bluetooth = variable_item_list_add(app->options, "Bluetooth", 2, NULL, app);
    VariableItem* set_kbl = variable_item_list_add(app->options, "Keyboard Layout", 1, NULL, app);
    UNUSED(bluetooth);
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
