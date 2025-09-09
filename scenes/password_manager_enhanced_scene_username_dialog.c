#include "../main.h"

void password_manager_enhanced_scene_username_dialog_callback(DialogExResult result, void* context) {
    AppContext* app = context;
    switch(result) {
    case DialogExResultLeft:
        scene_manager_next_scene(app->scene_manager, AppScene_password_menu);
        break;
    case DialogExResultCenter:
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_PasteUser);
        break;

    case DialogExResultRight:
        scene_manager_next_scene(app->scene_manager, AppScene_password_dialog);
        break;
    default:
        break;
    }
}

bool password_manager_enhanced_scene_username_dialog_on_event(
    void* context,
    SceneManagerEvent event) {
    AppContext* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case AppEvent_PasteUser:
            char* str = app->entry[app->entry_index].username;
            if(app->using_ble)
                ble_hid_paste(app, str);
            else
                usb_hid_paste(str);

            break;
        }
        break;
    case SceneManagerEventTypeBack:
        scene_manager_next_scene(app->scene_manager, AppScene_password_menu);
        break;
    default:
        consumed = false;
        break;
    }
    return consumed;
}

void password_manager_enhanced_scene_username_dialog_on_enter(void* context) {
    AppContext* app = context;
    dialog_ex_reset(app->dialog);
    dialog_ex_set_result_callback(
        app->dialog, password_manager_enhanced_scene_username_dialog_callback);

    dialog_ex_set_header(app->dialog, "Username:", 64, 16, AlignCenter, AlignCenter);
    dialog_ex_set_text(
        app->dialog, app->entry[app->entry_index].username, 64, 32, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(app->dialog, "Back");
    dialog_ex_set_center_button_text(app->dialog, "Paste");
    dialog_ex_set_right_button_text(app->dialog, "Pasw");
    dialog_ex_set_context(app->dialog, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_username_dialog);
}

void password_manager_enhanced_scene_username_dialog_on_exit(void* context) {
    AppContext* app = context;
    dialog_ex_reset(app->dialog);
}
