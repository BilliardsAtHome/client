#include "scene/SetupScene/SetupScene.h"

#include "core/Simulation.h"

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty.h>
#include <Pack/RPSystem.h>
#include <libkiwi.h>

namespace BAH {

K_SCENE_DECL(SetupScene);

/**
 * @brief Setup scene
 */
void SetupScene::OnConfigure() {
    // Setup game globals
    Setup();
    // Ask engine to start our async task
    setTaskAsync();

    // Reset display clear color to black
    RPGrpRenderer::SetEfbClearColor(0, 0, 0);
}

/**
 * @brief Update scene
 */
void SetupScene::OnCalculate() {
    // Wait for resources to finish loading
    if (!isTaskAsyncFinish()) {
        return;
    }

    // Skip login if unique ID already existed
    kiwi::SceneCreator::GetInstance().ChangeSceneAfterFade(
        Simulation::GetInstance().GetUniqueID() ? kiwi::ESceneID_RPBilScene
                                                : ESceneID_LoginScene);
}

/**
 * @brief Asynchronous tasks
 */
void SetupScene::taskAsync() {
    // Global archives
    RP_GET_INSTANCE(RPSysResourceManager)->LoadStaticArchives();
    RP_GET_INSTANCE(RPSysResourceManager)->LoadCacheArchives();

    // Global layouts
    RP_GET_INSTANCE(RPSysSystemWinMgr)->createSystemWindow();
    RP_GET_INSTANCE(RPSysTutorialWinMgr)->LoadResource();
    RP_GET_INSTANCE(RPSysPauseMgr)->LoadResource();
    RP_GET_INSTANCE(RPSysHomeMenuMgr)->LoadResource();

    // Miscellaneous resources
    RP_GET_INSTANCE(RPSysKokeshiManager)->LoadStaticResource();
    RP_GET_INSTANCE(RPSysEffectMgr)->LoadResource();
    RP_GET_INSTANCE(RPSysSaveDataMgr)->initBanner();
    RP_GET_INSTANCE(RPSysSystem)->createTimeStamp();
}

/**
 * @brief Setup game globals
 */
void SetupScene::Setup() {
    // Stop potential save data corruption
    RP_GET_INSTANCE(RPSysSaveDataMgr)->setSaveDisable(true);

    // Configure player manager for 1P
    RP_GET_INSTANCE(RPSysPlayerMgr)->resetData();
    RP_GET_INSTANCE(RPSysPlayerMgr)->setPlayerNum(1);
    RP_GET_INSTANCE(RPSysPlayerMgr)->setControllerNum(1);

    // Configure Party Pack game manager
    RPPartyGameMgr::CreateInstance();
    RP_GET_INSTANCE(RPPartyGameMgr)->Reset();

    // Create Billiards bruteforcer
    Simulation::CreateInstance();
}

} // namespace BAH