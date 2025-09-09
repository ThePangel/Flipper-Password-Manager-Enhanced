#pragma once

#include <gui/scene_manager.h>

#define ADD_SCENE(prefix, name, id) ExampleNumberInputScene##id,
typedef enum {
#include "password_manager_enhanced_scene_config.h"
    ExampleNumberInputSceneNum,
} ExampleNumberInputScene;
#undef ADD_SCENE

extern const SceneManagerHandlers password_manager_enhanced_input_scene_handlers;

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "password_manager_enhanced_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "password_manager_enhanced_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "password_manager_enhanced_scene_config.h"
#undef ADD_SCENE
