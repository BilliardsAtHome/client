#include "hooks/BilCue.h"

#include "core/Simulation.h"

#include <libkiwi.h>

namespace BAH {

/**
 * @brief Logic step
 */
void BilCue::CalculateEx() {
    mLastAimPosition = mAimPosition;
    mLastValidAim = mValidAim;
    mpStateMachine->Calculate();
}
KM_BRANCH_MF(0x802c0390, BilCue, CalculateEx);

/**
 * @brief HOLD state logic (extension)
 */
void BilCue::State_HOLD_calc_Ex() {
    // Move towards taking the shot
    ASSERT(mpStateMachine != NULL);
    mpStateMachine->ChangeState(EState_Wait);
}
KM_BRANCH_MF(0x802bf754, BilCue, State_HOLD_calc_Ex);

/**
 * @brief WAIT state logic (extension)
 */
void BilCue::State_WAIT_calc_Ex() {
    // Wait until game state expects input
    if (!RP_GET_INSTANCE(RPBilCtrlManager)->GetCtrl()->CanCtrl()) {
        return;
    }

    // Wait until simulation has finished aiming
    if (!Simulation::GetInstance().IsDoneAiming()) {
        return;
    }

    // Determine position in world-space
    CalcPosition();
    ASSERT(mValidAim);

    // Determine angular force onto ball
    CalcForce();

    // Move towards taking the shot
    ASSERT(mpStateMachine != NULL);
    mpStateMachine->ChangeState(EState_Pull);
}
KM_BRANCH_MF(0x802bf9d4, BilCue, State_WAIT_calc_Ex);

namespace {

/**
 * @brief Force PULL -> HIT transition
 */
KM_WRITE_32(0x802be88c, 0x4800002C);
KM_WRITE_32(0x802be9a0, 0x60000000);
KM_WRITE_32(0x802be9ac, 0x60000000);
KM_WRITE_32(0x802be9b8, 0x60000000);
KM_WRITE_32(0x802bead0, 0x60000000);

/**
 * @brief Redirect cue power calculation to the simulated value
 */
f32 bil_cue_get_power() {
    return Simulation::GetInstance().GetCuePower();
}
KM_CALL(0x802beff8, bil_cue_get_power);
KM_CALL(0x802bf070, bil_cue_get_power);
KM_CALL(0x802bf080, bil_cue_get_power);

} // namespace
} // namespace BAH