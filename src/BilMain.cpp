#include "BilMain.hpp"

#include "Simulation.hpp"

#include <Pack/RPSystem.h>

namespace bah {

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
    RPSysSceneCreator::getInstance()->changeSceneAfterFade(
        RPSysSceneCreator::ESceneID_RPBilScene);
}

/**
 * @brief Enter AFTERSHOT state
 */
void BilMain::State_AFTERSHOT_enter() {
    OnEndShot();
}
KM_BRANCH_MF(0x802c57ec, BilMain, State_AFTERSHOT_enter);

/**
 * @brief Enter FOUL state
 */
void BilMain::State_FOUL_enter() {
    OnEndShot();
}
KM_BRANCH_MF(0x802c563c, BilMain, State_FOUL_enter);

} // namespace bah