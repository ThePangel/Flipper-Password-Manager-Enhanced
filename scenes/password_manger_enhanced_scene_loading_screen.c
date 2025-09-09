#include "../main.h"

void password_manager_enhanced_scene_loading_screen_on_enter(void* context) {
    AppContext* app = context;
    widget_reset(app->loading);
    widget_add_string_element(
        app->loading, 64, 32, AlignCenter, AlignCenter, FontPrimary, "CSV parsing: Counting");
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Loading);
}

bool password_manager_enhanced_scene_loading_screen_on_event(
    void* context,
    SceneManagerEvent event) {
    AppContext* app = context;
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

void password_manager_enhanced_scene_loading_screen_on_exit(void* context) {
    AppContext* app = context;
    widget_reset(app->loading);
}
