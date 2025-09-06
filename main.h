#pragma once

#include <gui/gui.h>
#include <gui/icon_i.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/menu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/widget.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <furi_hal_bt.h>
#include <extra_profiles/hid_profile.h>
#include <bt/bt_service/bt.h>
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>

#define HID_BT_KEYS_STORAGE_NAME ".bt_hid.keys"

typedef enum {
    AppScene_MainMenu,
    AppScene_Submenu,
    AppScene_loading,
    AppScene_password_menu,
    AppScene_options,
    AppScene_count,

} AppScene;

typedef struct {
    char* name;
    char* username;
    char* password;
} CsvEntry;

typedef struct {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Submenu* menu;
    Submenu* submenu;
    Submenu* password_menu;
    Widget* loading;
    DialogEx* dialog;
    DialogsApp* dialogs;
    VariableItemList* options;
    FuriString* file_path;
    FuriString* file_name;
    CsvEntry* entry;
    Bt* bt;
    FuriHalBleProfileBase* ble;
    bool using_ble;
    size_t entry_count;
    uint16_t entry_index;

} AppContext;

typedef enum {
    AppView_MainSMenu,
    AppView_Submenu,
    AppView_Loading,
    AppView_password_menu,
    AppView_dialog,
    AppView_options,
} AppView;

typedef enum {
    AppEvent_FileBrowser,
    AppEvent_PastePasswrd,
} AppEvent;

const uint16_t test[] = {
    HID_KEYBOARD_NONE, // NUL
    HID_KEYBOARD_NONE, // SOH
    HID_KEYBOARD_NONE, // STX
    HID_KEYBOARD_NONE, // ETX
    HID_KEYBOARD_NONE, // EOT
    HID_KEYBOARD_NONE, // ENQ
    HID_KEYBOARD_NONE, // ACK
    HID_KEYBOARD_NONE, // BEL
    HID_KEYBOARD_DELETE, // BS   Backspace
    HID_KEYBOARD_TAB, // TAB  Tab
    HID_KEYBOARD_RETURN, // LF   Enter
    HID_KEYBOARD_NONE, // VT
    HID_KEYBOARD_NONE, // FF
    HID_KEYBOARD_NONE, // CR
    HID_KEYBOARD_NONE, // SO
    HID_KEYBOARD_NONE, // SI
    HID_KEYBOARD_NONE, // DEL
    HID_KEYBOARD_NONE, // DC1
    HID_KEYBOARD_NONE, // DC2
    HID_KEYBOARD_NONE, // DC3
    HID_KEYBOARD_NONE, // DC4
    HID_KEYBOARD_NONE, // NAK
    HID_KEYBOARD_NONE, // SYN
    HID_KEYBOARD_NONE, // ETB
    HID_KEYBOARD_NONE, // CAN
    HID_KEYBOARD_NONE, // EM
    HID_KEYBOARD_NONE, // SUB
    HID_KEYBOARD_NONE, // ESC
    HID_KEYBOARD_NONE, // FS
    HID_KEYBOARD_NONE, // GS
    HID_KEYBOARD_NONE, // RS
    HID_KEYBOARD_NONE, // US
    HID_KEYBOARD_SPACEBAR, // ' ' Space
    HID_KEYBOARD_1 | KEY_MOD_LEFT_SHIFT, // !
    HID_KEYBOARD_APOSTROPHE | KEY_MOD_LEFT_SHIFT, // "
    HID_KEYBOARD_3 | KEY_MOD_RIGHT_ALT, // #
    HID_KEYBOARD_4 | KEY_MOD_LEFT_SHIFT, // $
    HID_KEYBOARD_5 | KEY_MOD_LEFT_SHIFT, // %
    HID_KEYBOARD_7 | KEY_MOD_LEFT_SHIFT, // &
    HID_KEYBOARD_APOSTROPHE, // '
    HID_KEYBOARD_9 | KEY_MOD_LEFT_SHIFT, // (
    HID_KEYBOARD_0 | KEY_MOD_LEFT_SHIFT, // )
    HID_KEYBOARD_8 | KEY_MOD_LEFT_SHIFT, // *
    HID_KEYBOARD_EQUAL_SIGN | KEY_MOD_LEFT_SHIFT, // +
    HID_KEYBOARD_COMMA, // ,
    HID_KEYBOARD_MINUS, // -
    HID_KEYBOARD_DOT, // .
    HID_KEYBOARD_7 | KEY_MOD_LEFT_SHIFT, // /
    HID_KEYBOARD_0, // 0
    HID_KEYBOARD_1, // 1
    HID_KEYBOARD_2, // 2
    HID_KEYBOARD_3, // 3
    HID_KEYBOARD_4, // 4
    HID_KEYBOARD_5, // 5
    HID_KEYBOARD_6, // 6
    HID_KEYBOARD_7, // 7
    HID_KEYBOARD_8, // 8
    HID_KEYBOARD_9, // 9
    HID_KEYBOARD_SEMICOLON | KEY_MOD_LEFT_SHIFT, // :
    HID_KEYBOARD_SEMICOLON, // ;
    HID_KEYBOARD_COMMA | KEY_MOD_LEFT_SHIFT, // <
    HID_KEYBOARD_EQUAL_SIGN, // =
    HID_KEYBOARD_DOT | KEY_MOD_LEFT_SHIFT, // >
    HID_KEYBOARD_SLASH | KEY_MOD_LEFT_SHIFT, // ?
    HID_KEYBOARD_2 | KEY_MOD_RIGHT_ALT, // @
    HID_KEYBOARD_A | KEY_MOD_LEFT_SHIFT, // A
    HID_KEYBOARD_B | KEY_MOD_LEFT_SHIFT, // B
    HID_KEYBOARD_C | KEY_MOD_LEFT_SHIFT, // C
    HID_KEYBOARD_D | KEY_MOD_LEFT_SHIFT, // D
    HID_KEYBOARD_E | KEY_MOD_LEFT_SHIFT, // E
    HID_KEYBOARD_F | KEY_MOD_LEFT_SHIFT, // F
    HID_KEYBOARD_G | KEY_MOD_LEFT_SHIFT, // G
    HID_KEYBOARD_H | KEY_MOD_LEFT_SHIFT, // H
    HID_KEYBOARD_I | KEY_MOD_LEFT_SHIFT, // I
    HID_KEYBOARD_J | KEY_MOD_LEFT_SHIFT, // J
    HID_KEYBOARD_K | KEY_MOD_LEFT_SHIFT, // K
    HID_KEYBOARD_L | KEY_MOD_LEFT_SHIFT, // L
    HID_KEYBOARD_M | KEY_MOD_LEFT_SHIFT, // M
    HID_KEYBOARD_N | KEY_MOD_LEFT_SHIFT, // N
    HID_KEYBOARD_O | KEY_MOD_LEFT_SHIFT, // O
    HID_KEYBOARD_P | KEY_MOD_LEFT_SHIFT, // P
    HID_KEYBOARD_Q | KEY_MOD_LEFT_SHIFT, // Q
    HID_KEYBOARD_R | KEY_MOD_LEFT_SHIFT, // R
    HID_KEYBOARD_S | KEY_MOD_LEFT_SHIFT, // S
    HID_KEYBOARD_T | KEY_MOD_LEFT_SHIFT, // T
    HID_KEYBOARD_U | KEY_MOD_LEFT_SHIFT, // U
    HID_KEYBOARD_V | KEY_MOD_LEFT_SHIFT, // V
    HID_KEYBOARD_W | KEY_MOD_LEFT_SHIFT, // W
    HID_KEYBOARD_X | KEY_MOD_LEFT_SHIFT, // X
    HID_KEYBOARD_Y | KEY_MOD_LEFT_SHIFT, // Y
    HID_KEYBOARD_Z | KEY_MOD_LEFT_SHIFT, // Z
    HID_KEYBOARD_OPEN_BRACKET, // [
    HID_KEYBOARD_BACKSLASH, // bslash
    HID_KEYBOARD_CLOSE_BRACKET, // ]
    HID_KEYBOARD_6 | KEY_MOD_LEFT_SHIFT, // ^
    HID_KEYBOARD_MINUS | KEY_MOD_LEFT_SHIFT, // _
    HID_KEYBOARD_GRAVE_ACCENT, // `
    HID_KEYBOARD_A, // a
    HID_KEYBOARD_B, // b
    HID_KEYBOARD_C, // c
    HID_KEYBOARD_D, // d
    HID_KEYBOARD_E, // e
    HID_KEYBOARD_F, // f
    HID_KEYBOARD_G, // g
    HID_KEYBOARD_H, // h
    HID_KEYBOARD_I, // i
    HID_KEYBOARD_J, // j
    HID_KEYBOARD_K, // k
    HID_KEYBOARD_L, // l
    HID_KEYBOARD_M, // m
    HID_KEYBOARD_N, // n
    HID_KEYBOARD_O, // o
    HID_KEYBOARD_P, // p
    HID_KEYBOARD_Q, // q
    HID_KEYBOARD_R, // r
    HID_KEYBOARD_S, // s
    HID_KEYBOARD_T, // t
    HID_KEYBOARD_U, // u
    HID_KEYBOARD_V, // v
    HID_KEYBOARD_W, // w
    HID_KEYBOARD_X, // x
    HID_KEYBOARD_Y, // y
    HID_KEYBOARD_Z, // z
    HID_KEYBOARD_OPEN_BRACKET | KEY_MOD_LEFT_SHIFT, // {
    HID_KEYBOARD_BACKSLASH | KEY_MOD_LEFT_SHIFT, // |
    HID_KEYBOARD_CLOSE_BRACKET | KEY_MOD_LEFT_SHIFT, // }
    HID_KEYBOARD_GRAVE_ACCENT | KEY_MOD_LEFT_SHIFT, // ~
    HID_KEYBOARD_NONE, // DEL
};

void app_menu_password_dialog_callback(void* context, uint32_t index);
void app_menu_username_dialog_callback(void* context, uint32_t index);
