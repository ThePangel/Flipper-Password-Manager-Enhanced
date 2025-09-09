#include "../main.h"

void password_manager_enhanced_main_menu_callback(void* context, uint32_t index) {
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

void password_manager_enhanced_scene_main_menu_on_enter(void* context) {
    AppContext* app = context;
    submenu_reset(app->menu);
    submenu_add_item(
        app->menu, "View passwords", 0, password_manager_enhanced_main_menu_callback, app);
    submenu_add_item(app->menu, "Options", 1, password_manager_enhanced_main_menu_callback, app);
    submenu_add_item(app->menu, "About", 2, password_manager_enhanced_main_menu_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_MainSMenu);
}

bool password_manager_enhanced_scene_main_menu_on_event(void* context, SceneManagerEvent event) {
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
                password_manager_enhanced_handle_csv(app);
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

void password_manager_enhanced_scene_main_menu_on_exit(void* context) {
    AppContext* app = context;
    submenu_reset(app->menu);
}
