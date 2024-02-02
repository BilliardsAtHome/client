#include "BilMain.hpp"

#include "BilRunner.hpp"

#include <Pack/RPSystem.h>

/**
 * @brief Shorten intro camera
 */
KM_WRITE_32(0x802c5b30, 0x60000000);

/**
 * @brief Reset game state
 */
void BilMain::OnEndShot() {
    BilRunner::GetInstance().OnEndShot();

    RPSysSceneCreator::getInstance()->changeSceneAfterFade(
        RPSysSceneCreator::RP_BIL_SCENE);
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