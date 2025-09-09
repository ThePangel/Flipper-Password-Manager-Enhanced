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

#include "scenes/password_manager_enhanced_scene.h"
#include "password_manager_enhanced_icons.h"

#define CSV_LOAD_PATH            APP_DATA_PATH("")
#define KBL_LOAD_PATH            APP_ASSETS_PATH("")
#define HID_BT_KEYS_STORAGE_NAME ".bt_hid.keys"

typedef enum {
    AppScene_MainMenu,
    AppScene_Submenu,
    AppScene_loading,
    AppScene_password_menu,
    AppScene_options,
    AppScene_username_dialog,
    AppScene_password_dialog,
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
    AppView_username_dialog,
    AppView_password_dialog,
    AppView_options,
} AppView;

typedef enum {
    AppEvent_FileBrowser,
    AppEvent_PasteUser,
    AppEvent_PastePasswrd,
    AppEvent_PasteAll,

} AppEvent;

#define USB_ASCII_TO_KEY(x) (((uint8_t)x < 128) ? (hid_asciimap[(uint8_t)x]) : HID_KEYBOARD_NONE)

void ble_hid_paste(void* context, char* str);
void usb_hid_paste(char* str);
void password_manager_enhanced_handle_csv(void* context);
