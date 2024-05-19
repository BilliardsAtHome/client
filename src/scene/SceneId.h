#ifndef BAH_CLIENT_SCENE_SCENE_ID_H
#define BAH_CLIENT_SCENE_SCENE_ID_H
#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Billiards@home scene ID list
 */
enum ESceneID {
    ESceneID_SetupScene = kiwi::ESceneID_Max,
    ESceneID_LoginScene,
};

} // namespace BAH

#endif