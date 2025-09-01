#include <furi.h>
#include "main.h"
#include <gui/gui.h>

bool app_scene_on_event_main_menu(void* context, SceneManagerEvent event) {
    AppContext* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case AppEvent_ShowPasswrds:
            scene_manager_next_scene(app->scene_manager, AppScene_Submenu);
            consumed = true;
            break;
        case AppEvent_FileBrowser:
            scene_manager_next_scene(app->scene_manager, AppScene_Submenu);
            consumed = true;
            break;
        }
        break;

    default:
        consumed = false;
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
        case AppEvent_ShowPasswrds:
            scene_manager_next_scene(app->scene_manager, AppScene_Submenu);
            consumed = true;
            break;
        case AppEvent_FileBrowser:
            scene_manager_next_scene(app->scene_manager, AppScene_Submenu);
            consumed = true;
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
        scene_manager_handle_custom_event(app->scene_manager, AppEvent_ShowPasswrds);
        break;
    }
}

void app_menu_callback_sub_menu(void* context, uint32_t index) {
    AppContext* app = context;
    switch(index) {
    case 0:
        scene_manager_handle_custom_event(app->scene_manager, AppEvent_ShowPasswrds);
        break;
    }
}

void app_scene_on_enter_main_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->menu);
    submenu_add_item(app->menu, "View passwords", 0, app_menu_callback_sub_menu, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_MainSMenu);
}
void app_scene_on_enter_sub_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->submenu);
    submenu_add_item(app->submenu, "Test", 0, app_menu_callback_sub_menu, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Submenu);
}
void app_scene_on_exit_sub_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->submenu);
}

void app_scene_on_exit_main_menu(void* context) {
    AppContext* app = context;
    submenu_reset(app->menu);
}

void (*const app_scene_on_enter_handlers[])(void*) = {
    app_scene_on_enter_main_menu,
    app_scene_on_enter_sub_menu};

bool (*const app_scene_on_event_handlers[])(void*, SceneManagerEvent) = {
    app_scene_on_event_main_menu,
    app_scene_on_event_sub_menu,
};

void (*const app_scene_on_exit_handlers[])(void*) = {
    app_scene_on_exit_main_menu,
    app_scene_on_exit_sub_menu,
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
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    return app;
}

void app_free(void* context) {
    AppContext* app = context;

    scene_manager_free(app->scene_manager);

    view_dispatcher_remove_view(app->view_dispatcher, AppView_MainSMenu);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Submenu);

    view_dispatcher_free(app->view_dispatcher);

    submenu_free(app->menu);
    submenu_free(app->submenu);

    furi_record_close(RECORD_DIALOGS);
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
