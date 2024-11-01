#ifndef BAH_CLIENT_CORE_SIMULATION_H
#define BAH_CLIENT_CORE_SIMULATION_H
#include "core/BreakInfo.h"

#include <Pack/RPGraphics.h>
#include <Pack/RPParty.h>
#include <libkiwi.h>
#include <types.h>

namespace BAH {

// Forward declarations
struct BreakInfo;

/**
 * @brief Billiards simulation runner
 */
class Simulation : public kiwi::DynamicSingleton<Simulation>,
                   public kiwi::ISceneHook,
                   public IRPGrpDrawObject {
    friend class kiwi::DynamicSingleton<Simulation>;

public:
    /**
     * @brief Logic before scene reset
     */
    void BeforeReset();
    /**
     * @brief Logic after scene reset
     */
    void AfterReset();

    /**
     * @brief Update logic
     */
    void Tick();
    /**
     * @brief Commits break results
     */
    void Finish();

    /**
     * @brief Accesses the user's unique ID
     */
    kiwi::Optional<u32> GetUniqueID() const {
        return mUniqueID;
    }
    /**
     * @brief Sets the user's unique ID
     *
     * @param id Unique ID
     */
    void SetUniqueID(u32 id) {
        mUniqueID = id;
    }

    /**
     * @brief Tests whether the cue has finished aiming
     */
    bool IsAimFinish() const {
        return mTimerUp <= 0 && mTimerLeft <= 0 && mTimerRight <= 0;
    }

    /**
     * @brief Accesses the cue's shot power
     */
    f32 GetCuePower() const {
        return mpCurrBreak->power;
    }

    /**
     * @brief Tests whether this simulation is the first break of the session
     */
    bool IsFirstRun() const {
        return mIsFirstRun;
    }
    /**
     * @brief Tests whether this simulation is a replay
     */
    bool IsReplay() const {
        return mIsReplay;
    }
    /**
     * @brief Tests whether this simulation has finished
     */
    bool IsFinished() const {
        return mIsFinished;
    }

private:
    /**
     * @brief Randomization style
     */
    enum EStyle {
        EStyle_Normal, //!< Standard break
        EStyle_Jump,   //!< Jump the ball

        EStyle_Max
    };

    //! Horizontal turn speed
    static const f32 TURN_SPEED_X;
    //! Vertical turn speed
    static const f32 TURN_SPEED_Y;

    //! Maximum cue power
    static const f32 POWER_MAX;

private:
    /**
     * @brief Constructor
     */
    Simulation();
    /**
     * @brief Destructor
     */
    virtual ~Simulation();

    /**
     * @brief Configure callback
     *
     * @param pScene Current scene
     */
    virtual void Configure(RPSysScene* pScene);
    /**
     * @brief Standard draw pass
     */
    virtual void UserDraw();

    /**
     * @brief Loads user info (from DVD or NAND)
     */
    void LoadUser();
    /**
     * @brief Loads break info (from NAND)
     */
    void LoadBreak();

private:
    //! User unique ID
    kiwi::Optional<u32> mUniqueID;
    //! Server connection status
    kiwi::Optional<bool> mIsConnected;

    //! Last HTTP error
    kiwi::EHttpErr mHttpError;
    //! Last HTTP extended error
    s32 mHttpExError;
    //! Last HTTP status code
    kiwi::EHttpStatus mHttpStatus;

    //! Frames to aim up
    s32 mTimerUp;
    //! Frames to aim left
    s32 mTimerLeft;
    //! Frames to aim right
    s32 mTimerRight;

    //! Current break information
    BreakInfo* mpCurrBreak;
    //! Best break information
    BreakInfo* mpBestBreak;

    //! Whether this is the first break
    bool mIsFirstRun;
    //! Whether this is the first scene tick
    bool mIsFirstTick;

    //! Whether this break is a replay
    bool mIsReplay;
    //! Whether this break has finished
    bool mIsFinished;

    //! Total number of breaks
    u32 mBreakNum;
    //! Total number of breaks by ball count
    u32 mBreakBallNum[RPBilBallManager::BALL_MAX];
};

} // namespace BAH

#endif