#ifndef BAH_CLIENT_SCENE_LOGIN_SCENE_H
#define BAH_CLIENT_SCENE_LOGIN_SCENE_H
#include "scene/LoginScene/Keypad.h"
#include "scene/SceneId.h"

#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief User login scene
 */
class LoginScene : public kiwi::IScene {
public:
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
        return kiwi::EExitType_Sibling;
    }

    /**
     * @brief Setup scene
     */
    virtual void OnConfigure();
    /**
     * @brief Reset scene
     */
    virtual void OnReset();
    /**
     * @brief Update scene
     */
    virtual void OnCalculate();
    /**
     * @brief Standard draw pass
     */
    virtual void OnUserDraw();

private:
    /**
     * @brief Keypad submit ('OK') callback
     *
     * @param rResult Keypad result sequence
     * @param pArg User argument
     */
    static void KeypadOkCallback(const kiwi::String& rResult, void* pArg);

private:
    //! Unique ID keypad
    Keypad mKeypad;
};

} // namespace BAH

#endif
