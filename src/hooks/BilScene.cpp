#include "hooks/BilScene.h"

#include "core/Simulation.h"

#include <Pack/RPParty.h>
#include <revolution/DSP.h>

namespace BAH {

/**
 * @brief Scene logic
 */
void BilScene::CalculateEx() {
    // Wait for the initial fade-in
    if (!isSceneDisplay()) {
        return;
    }

    // Replay runs alongside framerate
    if (Simulation::GetInstance().IsReplay()) {
        Simulation::GetInstance().Tick();
        RPBilMain::GetInstance()->Calculate();
        return;
    }

    // Stop context switches
    kiwi::AutoInterruptLock lock;
    // Reset DSP to mute "crash sound"
    DSP_HW_REGS[DSP_CSR] |= 0x1;

    // Prepare the break
    Simulation::GetInstance().BeforeReset();
    RPBilMain::GetInstance()->Reset();
    Simulation::GetInstance().AfterReset();

    // Simulate the entire break
    while (!Simulation::GetInstance().IsFinished()) {
        Simulation::GetInstance().Tick();
        RPBilMain::GetInstance()->Calculate();
    }

    // Need to reset again if we will replay
    if (Simulation::GetInstance().IsReplay()) {
        Simulation::GetInstance().BeforeReset();
        RPBilMain::GetInstance()->Reset();
        Simulation::GetInstance().AfterReset();
    }
}
KM_BRANCH_MF(0x802ba1e0, BilScene, CalculateEx);

/**
 * @brief Remove "Press B" layout
 */
KM_WRITE_32(0x802d25f4, 0x4E800020);

/**
 * @brief Skip camera animations
 */
KM_WRITE_32(0x801bde3c, 0x48000100);

} // namespace BAH
