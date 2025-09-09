#include "password_manager_enhanced_scene.h"

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const password_manager_enhanced_input_on_enter_handlers[])(void*) = {
#include "password_manager_enhanced_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const password_manager_enhanced_input_on_event_handlers[])(
    void* context,
    SceneManagerEvent event) = {
#include "password_manager_enhanced_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const password_manager_enhanced_input_on_exit_handlers[])(void* context) = {
#include "password_manager_enhanced_scene_config.h"
};
#undef ADD_SCENE

const SceneManagerHandlers password_manager_enhanced_input_scene_handlers = {
    .on_enter_handlers = password_manager_enhanced_input_on_enter_handlers,
    .on_event_handlers = password_manager_enhanced_input_on_event_handlers,
    .on_exit_handlers = password_manager_enhanced_input_on_exit_handlers,
    .scene_num = ExampleNumberInputSceneNum,
};
