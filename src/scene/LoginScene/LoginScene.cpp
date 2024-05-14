#include "LoginScene.h"

#include <Pack/RPAudio.h>

namespace BAH {

K_SCENE_DECL(LoginScene);

/**
 * @brief Setup scene
 */
void LoginScene::OnConfigure() {
    mStateMachine.RegistState(State_NUMPAD_enter, State_NUMPAD_calc,
                              EState_NUMPAD);
    mStateMachine.RegistState(State_CONNECT_enter, State_CONNECT_calc,
                              EState_CONNECT);

    mKeypad.SetOkCallback(KeypadOkCallback, this);
}

/**
 * @brief Reload scene
 */
void LoginScene::OnReset() {
    mStateMachine.ChangeState(EState_NUMPAD);
}

/**
 * @brief Scene logic
 */
void LoginScene::OnCalculate() {
    mStateMachine.Calculate();
}

/**
 * @brief User-level draw
 */
void LoginScene::OnUserDraw() {
    if (mStateMachine.IsState(EState_NUMPAD)) {
        kiwi::DebugPrint::PrintfOutline(0.0f, 0.7f, 1.0f, true,
                                        kiwi::Color::WHITE, kiwi::Color::GREY,
                                        "Enter your unique ID:");

        kiwi::DebugPrint::PrintfOutline(
            0.0f, -0.8f, 1.0f, true, kiwi::Color::YELLOW, kiwi::Color::GREY,
            "Use the /get-id Discord command if you are unsure");

        mKeypad.UserDraw();
    }
}

/**
 * @brief Enter NUMPAD state
 */
void LoginScene::State_NUMPAD_enter() {
    mKeypad.Reset();
}
/**
 * @brief Update NUMPAD state
 */
void LoginScene::State_NUMPAD_calc() {
    mKeypad.Calculate();
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

/**
 * @brief Keypad OK keypress callback
 *
 * @param result Keypad result
 * @param arg Parent scene
 */
void LoginScene::KeypadOkCallback(const kiwi::String& result, void* arg) {
    LoginScene* self = static_cast<LoginScene*>(arg);
    ASSERT(self != NULL);

    K_LOG_EX("[SUBMIT] %s\n", result.CStr());
}

} // namespace BAH
