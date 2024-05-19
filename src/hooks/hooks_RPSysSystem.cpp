#include "scene/SceneId.h"

#include <kokeshi.hpp>
#include <libkiwi.h>

namespace BAH {
namespace {

/**
 * @brief Load setup scene instead of RP boot scene
 */
void change_boot_scene() {
    kiwi::SceneCreator::GetInstance().ChangeSceneAfterFade(ESceneID_SetupScene);
}
KM_CALL(0x80183784, change_boot_scene);

} // namespace
} // namespace BAH