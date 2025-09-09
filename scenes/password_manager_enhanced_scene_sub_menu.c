#include "../main.h"

void password_manager_enhanced_scene_sub_menu_callback(void* context, uint32_t index) {
    AppContext* app = context;
    app->entry_index = index;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void password_manager_enhanced_scene_sub_menu_on_enter(void* context) {
    AppContext* app = context;
    submenu_reset(app->submenu);
    for(size_t i = 0; i < app->entry_count; i++) {
        submenu_add_item(
            app->submenu,
            app->entry[i].name,
            i,
            password_manager_enhanced_scene_sub_menu_callback,
            app);
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Submenu);
}

bool password_manager_enhanced_scene_sub_menu_on_event(void* context, SceneManagerEvent event) {
    AppContext* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case 0:
            scene_manager_next_scene(app->scene_manager, AppScene_password_menu);
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
void password_manager_enhanced_scene_sub_menu_on_exit(void* context) {
    AppContext* app = context;
    submenu_reset(app->submenu);
}
