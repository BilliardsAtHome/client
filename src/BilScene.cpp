#include "BilRunner.hpp"

#include <Pack/RPParty.h>

static bool IsSceneFadeOut() {
    return RPSysSceneMgr::getInstance()->getSceneStatus() ==
           EGG::Fader::STATUS_FADE_OUT;
}

static bool IsSceneFadeIn() {
    return RPSysSceneMgr::getInstance()->getSceneStatus() ==
           EGG::Fader::STATUS_FADE_IN;
}

static void OnCalculate(RPSysScene* scene) {
    // Run one tick as usual
    BilRunner::GetInstance().Simulate();
    RPBilMain::GetInstance()->Calculate();

    // Replay -> Just one tick
    if (BilRunner::GetInstance().IsReplay()) {
        return;
    }

    // Not replay -> simulate without ties to graphics/framerate
    // BOOL enabled = OSDisableInterrupts();
    while (scene->isSceneDisplay()) {
        BilRunner::GetInstance().Simulate();
        RPBilMain::GetInstance()->Calculate();
    }
    // OSRestoreInterrupts(enabled);
}
KM_BRANCH(0x802ba1e0, OnCalculate);
