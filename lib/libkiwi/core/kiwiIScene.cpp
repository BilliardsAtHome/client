#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <libkiwi.h>

namespace kiwi {

/**
 * @brief Setup scene
 */
void IScene::Configure() {
    // Setup model scene
    RPGrpModelResManager::GetInstance()->CreateResourceList(0x400);
    RPGrpModelManager::GetInstance()->CreateModelScene(0, 0, 1, NULL, NULL);

    // Setup renderer view
    RPGrpScreen* screen = new RPGrpScreen();
    screen->SetCanvasMode(RPGrpScreen::CANVASMODE_0);
    RPGrpRenderer::GetInstance()->CreateView2D(1, screen);
    RPGrpRenderer::GetInstance()->CorrectView();

    // User state function
    OnConfigure();
}

/**
 * @brief Load required assets
 */
void IScene::LoadResource() {
    // User state function
    OnLoadResource();
}

/**
 * @brief Reload scene
 */
void IScene::Reset() {
    RPSysCursorDrawMgr::GetInstance()->startDpdCheck();
    RPSysCursorDrawMgr::GetInstance()->createActiveCursor();

    // User state function
    OnReset();
}

/**
 * @brief Scene logic
 */
void IScene::Calculate() {
    // User state function
    OnCalculate();
}

#ifdef PACK_RESORT
/**
 * @brief Scene MotionPlus logic
 */
void IScene::CalculateMPlus() {
    // User state function
    OnCalculateMPlus();
}
#endif

/**
 * @brief Exit scene
 */
void IScene::Exit() {
    // User state function
    OnExit();
}

/**
 * @brief User-level draw
 */
void IScene::UserDraw() {
    // User state function
    OnUserDraw();
}

/**
 * @brief Debug-level draw
 */
void IScene::DebugDraw() {
    // User state function
    OnDebugDraw();
}

} // namespace kiwi