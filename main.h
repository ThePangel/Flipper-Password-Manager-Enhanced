#pragma once

#include <gui/gui.h>
#include <gui/icon_i.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/menu.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/widget.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

typedef enum {
    AppScene_MainMenu,
    AppScene_Submenu,
    AppScene_loading,
    AppScene_password_menu,
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
    FuriString* file_path;
    FuriString* file_name;
    CsvEntry* entry;
    size_t entry_count;
    uint16_t entry_index;

} AppContext;

typedef enum {
    AppView_MainSMenu,
    AppView_Submenu,
    AppView_Loading,
    AppView_password_menu,
    AppView_dialog,
} AppView;

typedef enum {
    AppEvent_FileBrowser,
    AppEvent_LoadPasswrd,
} AppEvent;
