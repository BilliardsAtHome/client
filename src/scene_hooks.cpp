#include "Simulation.hpp"

#include <Pack/RPParty.h>

namespace bah {
namespace {

void Scene_Calculate(RPSysScene* scene) {
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
KM_BRANCH(0x802ba1e0, Scene_Calculate);

} // namespace
} // namespace bah
