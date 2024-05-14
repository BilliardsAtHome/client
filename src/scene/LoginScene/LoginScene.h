#ifndef BAH_CLIENT_SCENE_LOGIN_SCENE_H
#define BAH_CLIENT_SCENE_LOGIN_SCENE_H
#include "scene/LoginScene/Keypad.h"
#include "scene/SceneId.h"

#include <Pack/RPUtility.h>
#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief User login scene
 */
class LoginScene : public kiwi::IScene {
public:
    enum EState {
        EState_NUMPAD,
        EState_CONNECT,

        EState_Max
    };

    RP_UTL_FSM_STATE_DECL(NUMPAD);
    RP_UTL_FSM_STATE_DECL(CONNECT);

public:
    /**
     * @brief Constructor
     */
    LoginScene() : mStateMachine(this, EState_Max) {}
    /**
     * @brief Destructor
     */
    virtual ~LoginScene() {}

    /**
     * @brief Get the scene's name
     */
    virtual kiwi::String GetName() const {
        return "User Login";
    }
    /**
     * @brief Get the scene's resource directory name
     */
    virtual kiwi::String GetDirectory() const {
        return "LoginScene/";
    }
    /**
     * @brief Get the scene's ID
     */
    virtual s32 GetID() const {
        return ESceneID_LoginScene;
    }
    /**
     * @brief Get the scene's exit type
     */
    virtual kiwi::EExitType GetExitType() const {
        return kiwi::EExitType_1;
    }

    /**
     * @brief Setup scene
     */
    virtual void OnConfigure();
    /**
     * @brief Reload scene
     */
    virtual void OnReset();
    /**
     * @brief Scene logic
     */
    virtual void OnCalculate();
    /**
     * @brief User-level draw
     */
    virtual void OnUserDraw();

private:
    static void KeypadOkCallback(const kiwi::String& result, void* arg);

private:
    RPUtlBaseFsm<LoginScene> mStateMachine; // Scene state
    Keypad mKeypad;                         // Unique ID keypad
};

} // namespace BAH

#endif