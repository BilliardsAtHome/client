#ifndef BAH_CLIENT_SCENE_SETUP_SCENE_H
#define BAH_CLIENT_SCENE_SETUP_SCENE_H
#include "scene/SceneId.h"

#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Game setup scene
 */
class SetupScene : public kiwi::IScene {
public:
    /**
     * @brief Get the scene's name
     */
    virtual kiwi::String GetName() const {
        return "Setup";
    }
    /**
     * @brief Get the scene's resource directory name
     */
    virtual kiwi::String GetDirectory() const {
        return "SetupScene/";
    }
    /**
     * @brief Get the scene's ID
     */
    virtual s32 GetID() const {
        return ESceneID_SetupScene;
    }
    /**
     * @brief Get the scene's create type
     */
    virtual kiwi::ECreateType GetCreateType() const {
        return kiwi::ECreateType_Standard;
    }
    /**
     * @brief Get the scene's exit type
     */
    virtual kiwi::EExitType GetExitType() const {
        return kiwi::EExitType_3;
    }

    /**
     * @brief Setup scene
     */
    virtual void OnConfigure();
    /**
     * @brief Update scene
     */
    virtual void OnCalculate();

    /**
     * @brief Asynchronous tasks
     */
    virtual void taskAsync();

private:
    /**
     * @brief Setup game globals
     */
    void Setup();
};

} // namespace BAH

#endif