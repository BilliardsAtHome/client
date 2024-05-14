#include "hooks/BilMain.h"

#include "core/Simulation.h"

#include <Pack/RPSystem.h>
#include <libkiwi.h>

namespace BAH {

/**
 * @brief Shorten intro camera (OPENINGDEMO state)
 */
KM_WRITE_32(0x802c5b30, 0x60000000);

/**
 * @brief Shot end callback
 */
void BilMain::OnEndShot() {
    // Let simulation record results
    Simulation::GetInstance().OnEndShot();

    // Reload the billiards scene
    kiwi::SceneCreator::GetInstance().ChangeSceneAfterFade(
        kiwi::ESceneID_RPBilScene);
}
KM_BRANCH_MF(0x802c57ec, BilMain, OnEndShot);
KM_BRANCH_MF(0x802c563c, BilMain, OnEndShot);

} // namespace BAH