#include "hooks/PartyMenuScene.h"

#include "core/Simulation.h"
#include "scene/SceneId.h"

#include <libkiwi.h>

namespace BAH {

/**
 * @brief Scene logic
 */
void PartyMenuScene::CalculateEx() {
    // Skip login process if unique ID is saved on the NAND
    kiwi::SceneCreator::GetInstance().ChangeSceneAfterFade(
        Simulation::GetInstance().GetUniqueId().HasValue()
            ? kiwi::ESceneID_RPBilScene
            : ESceneID_LoginScene);
}
KM_BRANCH_MF(0x801e4388, PartyMenuScene, CalculateEx);

} // namespace BAH