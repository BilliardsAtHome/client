#include "hooks/BilScene.h"

#include "core/Simulation.h"

#include <Pack/RPParty.h>
#include <revolution/DSP.h>

namespace BAH {

/**
 * @brief Register any custom draw objects with the scene renderer
 */
void BilScene::RegistDrawObject() {
    RPGrpRenderer::GetInstance()->AppendDrawObject(&Simulation::GetInstance());
}
KM_BRANCH(0x802baa3c, BilScene::RegistDrawObject);

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
        RP_GET_INSTANCE(RPBilMain)->Calculate();
        return;
    }

    // Stop context switches
    kiwi::AutoInterruptLock lock;

    // Need to reset early if this is the first break
    if (Simulation::GetInstance().IsFirstRun()) {
        Simulation::GetInstance().BeforeReset();
        RP_GET_INSTANCE(RPBilMain)->Reset();
        Simulation::GetInstance().AfterReset();
    }

    // Simulate the entire break
    while (!Simulation::GetInstance().IsFinished()) {
        Simulation::GetInstance().Tick();
        RP_GET_INSTANCE(RPBilMain)->Calculate();
    }

    // Prepare for the next break
    Simulation::GetInstance().BeforeReset();
    RP_GET_INSTANCE(RPBilMain)->Reset();
    Simulation::GetInstance().AfterReset();
}
KM_BRANCH_MF(0x802ba1e0, BilScene, CalculateEx);

/**
 * @brief Remove "Press B" layout
 */
KM_WRITE_32(0x802d25f4, 0x4E800020);

} // namespace BAH
