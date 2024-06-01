#ifndef LIBKIWI_CORE_SCENE_HOOK_MGR_H
#define LIBKIWI_CORE_SCENE_HOOK_MGR_H
#include <Pack/RPSystem.h>
#include <libkiwi/core/kiwiSceneCreator.h>
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/prim/kiwiArray.h>
#include <libkiwi/prim/kiwiLinkList.h>
#include <libkiwi/util/kiwiStaticSingleton.h>

namespace kiwi {

// Forward declarations
class ISceneHook;

/**
 * @brief Scene hook manager
 */
class SceneHookMgr : public StaticSingleton<SceneHookMgr> {
    friend class StaticSingleton<SceneHookMgr>;

public:
    /**
     * @brief Registers new hook
     *
     * @param hook Scene hook
     * @param id Scene ID (-1 for all scenes)
     */
    void AddHook(ISceneHook& hook, s32 id);

    /**
     * @brief Unregisters existing hook
     *
     * @param hook Scene hook
     * @param id Scene ID (-1 for all scenes)
     */
    void RemoveHook(const ISceneHook& hook, s32 id);

private:
    LIBKIWI_KAMEK_PUBLIC

    /**
     * @brief Enter state
     */
    static void DoEnter();
    /**
     * @brief Reset state
     */
    static void DoReset();
    /**
     * @brief LoadResource state
     */
    static void DoLoadResource();
    /**
     * @brief Calculate state
     */
    static void DoCalculate();
    /**
     * @brief Exit state
     */
    static void DoExit();
    /**
     * @brief Pause state
     */
    static void DoPause();
    /**
     * @brief Un-pause state
     */
    static void DoUnPause();

    /**
     * @brief Gets list of hooks for the current scene
     */
    TList<ISceneHook>& GetActiveHooks();

private:
    TArray<TList<ISceneHook>, ESceneID_Max> mHookLists; // Lists of scene hooks
    TList<ISceneHook> mGlobalHooks; // Global hooks (always active)
};

/**
 * @brief Scene hook interface
 */
class ISceneHook {
public:
    /**
     * @brief Constructor
     *
     * @param id Scene ID (-1 for all scenes)
     */
    explicit ISceneHook(s32 id) : mSceneID(id) {
        K_ASSERT_EX(id == -1 || id < ESceneID_Max,
                    "Only RP scenes and -1 (all) are supported");

        SceneHookMgr::GetInstance().AddHook(*this, mSceneID);
    }

    /**
     * @brief Destructor
     */
    virtual ~ISceneHook() {
        SceneHookMgr::GetInstance().RemoveHook(*this, mSceneID);
    }

    /**
     * @brief Configure callback
     * @details Ran once on initial scene setup
     */
    virtual void Configure(RPSysScene* scene) {}

    /**
     * @brief LoadResource callback
     * @details Ran once on asset loading
     */
    virtual void LoadResource(RPSysScene* scene) {}

    /**
     * @brief Reset callback (before game logic)
     * @details Ran once on initial scene setup and on every restart
     */
    virtual void BeforeReset(RPSysScene* scene) {}
    /**
     * @brief Reset callback (after game logic)
     * @details Ran once on initial scene setup and on every restart
     */
    virtual void AfterReset(RPSysScene* scene) {}

    /**
     * @brief Calculate callback (before game logic)
     * @details Ran once per frame
     */
    virtual void BeforeCalculate(RPSysScene* scene) {}
    /**
     * @brief Calculate callback (after game logic)
     * @details Ran once per frame
     */
    virtual void AfterCalculate(RPSysScene* scene) {}

    /**
     * @brief Exit callback
     * @details Ran once on scene exit (including restarts)
     */
    virtual void Exit(RPSysScene* scene) {}

    /**
     * @brief Pause callback
     * @details Ran on pause menu open/close
     *
     * @param enter Whether the pause menu is opening
     */
    virtual void Pause(RPSysScene* scene, bool enter) {}

private:
    s32 mSceneID; // Scene to which this hook belongs
};

} // namespace kiwi

#endif
