#include "hooks/SysBootScene.h"

#include <libkiwi.h>

namespace BAH {

/**
 * @brief "Wait" state logic
 */
void SysBootScene::CalculateWait() {
    // Skip to the player select when resources are loaded
    if (isTaskAsyncFinish()) {
        kiwi::SceneCreator::GetInstance().ChangeSceneAfterFade(
            kiwi::ESceneID_RPSysPlayerSelectScene);
    }
}
KM_BRANCH_MF(0x801cb530, SysBootScene, CalculateWait);

} // namespace BAH