#ifndef BAH_CLIENT_SIMULATION_H
#define BAH_CLIENT_SIMULATION_H
#include "BreakInfo.h"

#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Billiards simulation runner
 */
class Simulation : public kiwi::DynamicSingleton<Simulation>,
                   public kiwi::ISceneHook {
    friend class kiwi::DynamicSingleton<Simulation>;

public:
    virtual void Configure(RPSysScene* scene);
    virtual void BeforeReset(RPSysScene* scene);
    virtual void AfterReset(RPSysScene* scene);

    void Tick();
    void OnEndShot();

    kiwi::Optional<u32> GetUniqueId() const {
        return mUniqueId;
    }
    void SetUniqueId(u32 id) {
        mUniqueId = id;
    }

    f32 GetCuePower() const {
        return mpCurrBreak->power;
    }

    bool IsReplay() const {
        return mIsReplay;
    }

    bool IsDoneAiming() const {
        return mTimerUp <= 0 && mTimerLeft <= 0 && mTimerRight <= 0;
    }

private:
    Simulation();
    virtual ~Simulation();

private:
    kiwi::Optional<u32> mUniqueId;
    int mTimerUp;
    int mTimerLeft;
    int mTimerRight;
    BreakInfo* mpCurrBreak;
    BreakInfo* mpBestBreak;
    bool mIsReplay;
    bool mIsFirstTick;
};

} // namespace BAH

#endif