#include "LoginScene.h"

namespace BAH {

K_SCENE_DECL(LoginScene);

/**
 * @brief Setup scene
 */
void LoginScene::Configure() {
    RPSndAudioMgr::GetInstance()->changeScene();

    mStateMachine.RegistState(State_NUMPAD_enter, State_NUMPAD_calc,
                              EState_NUMPAD);
    mStateMachine.RegistState(State_CONNECT_enter, State_CONNECT_calc,
                              EState_CONNECT);
}

/**
 * @brief Reload scene
 */
void LoginScene::Reset() {
    mStateMachine.ChangeState(EState_NUMPAD);
}

/**
 * @brief Scene logic
 */
void LoginScene::Calculate() {
    mStateMachine.Calculate();
}

/**
 * @brief Enter NUMPAD state
 */
void LoginScene::State_NUMPAD_enter() {
    ;
}
/**
 * @brief Update NUMPAD state
 */
void LoginScene::State_NUMPAD_calc() {
    ;
}

/**
 * @brief Enter CONNECT state
 */
void LoginScene::State_CONNECT_enter() {
    ;
}
/**
 * @brief Update CONNECT state
 */
void LoginScene::State_CONNECT_calc() {
    ;
}

} // namespace BAH
