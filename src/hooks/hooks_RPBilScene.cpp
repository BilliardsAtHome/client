#include "core/Simulation.h"

#include <Pack/RPParty.h>

namespace BAH {
namespace {

void bil_scene_calculate(RPSysScene* scene) {
    // Run one tick as usual
    Simulation::GetInstance().Tick();
    RPBilMain::GetInstance()->Calculate();

    // Replay -> Just one tick
    if (Simulation::GetInstance().IsReplay()) {
        return;
    }

    // Not replay -> simulate without ties to graphics/framerate
    while (scene->isSceneDisplay()) {
        Simulation::GetInstance().Tick();
        RPBilMain::GetInstance()->Calculate();
    }
}
KM_BRANCH(0x802ba1e0, bil_scene_calculate);

} // namespace
} // namespace BAH
