#ifndef BAH_CLIENT_SCENE_LOGIN_SCENE_H
#define BAH_CLIENT_SCENE_LOGIN_SCENE_H
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
     * @brief Get the scene's ID
     */
    virtual s32 GetID() const {
        return ESceneID_LoginScene;
    }

    /**
     * @brief Setup scene
     */
    virtual void Configure();
    /**
     * @brief Reload scene
     */
    virtual void Reset();
    /**
     * @brief Scene logic
     */
    virtual void Calculate();

private:
    RPUtlBaseFsm<LoginScene> mStateMachine;
};

} // namespace BAH

#endif