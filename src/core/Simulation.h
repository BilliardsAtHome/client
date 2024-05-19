#ifndef BAH_CLIENT_SIMULATION_H
#define BAH_CLIENT_SIMULATION_H
#include "BreakInfo.h"

#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Billiards simulation runner
 */
class Simulation : public kiwi::DynamicSingleton<Simulation> {
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

    f32 GetCuePower() const {
        return mpCurrBreak->power;
    }

    bool IsDoneAiming() const {
        return mTimerUp <= 0 && mTimerLeft <= 0 && mTimerRight <= 0;
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

    void LoadUniqueId();
    void LoadBestBreak();

private:
    kiwi::Optional<u32> mUniqueId;

    int mTimerUp;
    int mTimerLeft;
    int mTimerRight;

    BreakInfo* mpCurrBreak;
    BreakInfo* mpBestBreak;

    bool mIsFirstRun;
    bool mIsFirstTick;

    bool mIsReplay;
    bool mIsFinished;
};

} // namespace BAH

#endif