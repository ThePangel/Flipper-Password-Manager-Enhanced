#include "../main.h"

void password_manager_enhanced_scene_password_menu_callback(void* context, uint32_t index) {
    AppContext* app = context;
    switch(index) {
    case 0:
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_PasteUser);
        break;
    case 1:
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_PastePasswrd);
        break;
    case 2:
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_PasteAll);
        break;
    default:
        break;
    }
}

bool password_manager_enhanced_scene_password_menu_on_event(void* context, SceneManagerEvent event) {
    AppContext* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case AppEvent_PasteUser:
            scene_manager_next_scene(app->scene_manager, AppScene_username_dialog);
            break;
        case AppEvent_PastePasswrd:
            scene_manager_next_scene(app->scene_manager, AppScene_password_dialog);

            break;
        case AppEvent_PasteAll:
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
        }
        break;
    default:
        consumed = false;
        break;
    }
    return consumed;
}
void password_manager_enhanced_scene_password_menu_on_enter(void* context) {
    AppContext* app = context;
    size_t index = app->entry_index;
    submenu_reset(app->password_menu);
    submenu_reset(app->password_menu);
    submenu_set_header(app->password_menu, app->entry[index].name);
    char username[64];
    snprintf(username, sizeof(username), "Username: %s", app->entry[index].username);
    submenu_add_item(
        app->password_menu,
        username,
        0,
        password_manager_enhanced_scene_password_menu_callback,
        app);
    submenu_add_item(
        app->password_menu,
        "Reveal password",
        1,
        password_manager_enhanced_scene_password_menu_callback,
        app);
    submenu_add_item(
        app->password_menu,
        "Paste",
        2,
        password_manager_enhanced_scene_password_menu_callback,
        app);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_password_menu);
}

void password_manager_enhanced_scene_password_menu_on_exit(void* context) {
    AppContext* app = context;
    submenu_reset(app->password_menu);
}
