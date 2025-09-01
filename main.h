#pragma once

#include <gui/gui.h>
#include <gui/icon_i.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/menu.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <dialogs/dialogs.h>
typedef enum {
    AppScene_MainMenu,
    AppScene_Submenu,
    AppScene_count
} AppScene;

typedef struct {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Submenu* menu;
    Submenu* submenu;
    DialogsApp* dialogs;
} AppContext;

typedef enum {
    AppView_MainSMenu,
    AppView_Submenu
} AppView;

typedef enum {
    AppEvent_ShowPasswrds,
    AppEvent_FileBrowser,
} AppEvent;
