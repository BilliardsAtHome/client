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
    LoadAssets();
    SetupGame();
}

/**
 * @brief Reload scene
 */
void SetupScene::OnReset() {
    RP_GET_INSTANCE(RPPartyGameMgr)->Reset();

    // Reset display clear color to black
    RPGrpRenderer::SetEfbClearColor(0, 0, 0);

    // Skip login if UID existed as a file
    kiwi::SceneCreator::GetInstance().ChangeSceneAfterFade(
        Simulation::GetInstance().GetUniqueId() ? kiwi::ESceneID_RPBilScene
                                                : ESceneID_LoginScene);
}

/**
 * @brief Exit scene
 */
void SetupScene::OnExit() {
    RPPartyGameMgr::DestroyInstance();
}

/**
 * @brief Load static assets
 */
void SetupScene::LoadAssets() {
    // Global archives
    RP_GET_INSTANCE(RPSysResourceManager)->LoadStaticArchives();
    RP_GET_INSTANCE(RPSysResourceManager)->LoadCacheArchives();

    // Global layouts
    RP_GET_INSTANCE(RPSysSystemWinMgr)->createSystemWindow();
    RP_GET_INSTANCE(RPSysTutorialWinMgr)->LoadResource();
    RP_GET_INSTANCE(RPSysPauseMgr)->LoadResource();
    RP_GET_INSTANCE(RPSysHomeMenuMgr)->LoadResource();

    // Misc. resources
    RP_GET_INSTANCE(RPSysKokeshiManager)->LoadStaticResource();
    RP_GET_INSTANCE(RPSysEffectMgr)->LoadResource();
    RP_GET_INSTANCE(RPSysSaveDataMgr)->initBanner();
    RP_GET_INSTANCE(RPSysSystem)->createTimeStamp();
}

/**
 * @brief Setup game globals
 */
void SetupScene::SetupGame() {
    // Stop potential save data corruption
    RP_GET_INSTANCE(RPSysSaveDataMgr)->setSaveDisable(true);

    // Configure player manager for 1P
    RP_GET_INSTANCE(RPSysPlayerMgr)->resetData();
    RP_GET_INSTANCE(RPSysPlayerMgr)->setPlayerNum(1);
    RP_GET_INSTANCE(RPSysPlayerMgr)->setControllerNum(1);

    RPPartyGameMgr::CreateInstance();
    Simulation::CreateInstance();
}

} // namespace BAH