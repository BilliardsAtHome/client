#ifndef BAH_CLIENT_SIMULATION_H
#define BAH_CLIENT_SIMULATION_H
#include "BreakInfo.h"

#include <Pack/RPGraphics.h>
#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Billiards simulation runner
 */
class Simulation : public kiwi::DynamicSingleton<Simulation>,
                   public kiwi::ISceneHook,
                   public IRPGrpDrawObject {
    friend class kiwi::DynamicSingleton<Simulation>;

    /**
     * @brief Randomization style
     */
    enum EStyle {
        EStyle_Normal,
        EStyle_Jump,

        EStyle_Max
    };

public:
    kiwi::Optional<u32> GetUniqueId() const {
        return mUniqueId;
    }
    void SetUniqueId(u32 id) {
        mUniqueId = id;
    }

    bool IsDoneAiming() const {
        return mTimerUp <= 0 && mTimerLeft <= 0 && mTimerRight <= 0;
    }

    f32 GetCuePower() const {
        return mpCurrBreak->power;
    }

    bool IsFirstRun() const {
        return mIsFirstRun;
    }

    bool IsReplay() const {
        return mIsReplay;
    }

    bool IsFinished() const {
        return mIsFinished;
    }

    void BeforeReset();
    void AfterReset();

    void Tick();
    void Finish();

private:
    Simulation();
    virtual ~Simulation();

    virtual void Configure(RPSysScene* scene);
    virtual void UserDraw();

    void LoadUniqueId();
    void LoadBestBreak();

private:
    kiwi::Optional<u32> mUniqueId;
    kiwi::Optional<bool> mIsConnected;

    kiwi::EHttpErr mHttpError;
    kiwi::EHttpStatus mHttpStatus;

    int mTimerUp;
    int mTimerLeft;
    int mTimerRight;

    BreakInfo* mpCurrBreak;
    BreakInfo* mpBestBreak;

    bool mIsFirstRun;
    bool mIsFirstTick;

    bool mIsReplay;
    bool mIsFinished;

    u32 mNumBreak;
    u32 mNumBall[10];
};

} // namespace BAH

#endif