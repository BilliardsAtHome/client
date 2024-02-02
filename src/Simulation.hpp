#ifndef BAH_CLIENT_SIMULATION_H
#define BAH_CLIENT_SIMULATION_H
#include <libkiwi.h>
#include <types.h>

namespace bah {

/**
 * @brief Billiards simulation runner
 */
class Simulation : public kiwi::DynamicSingleton<Simulation>,
                   public kiwi::IScnHook {
    friend class kiwi::DynamicSingleton<Simulation>;

public:
    struct BreakInfo {
        u32 seed;
        int num;
        int frame;
        int aimU;
        int aimL;
        int aimR;
        EGG::Vector2f pos;
        f32 power;
        bool foul;
    };

public:
    virtual void Configure(RPSysScene* scene);
    virtual void BeforeReset(RPSysScene* scene);
    virtual void AfterReset(RPSysScene* scene);

    void Tick();
    void OnEndShot();

    f32 GetCuePower() const {
        return mIsReplay ? mpBestBreak->power : mCuePower;
    }

    bool IsReplay() const {
        return mIsReplay;
    }

    bool IsAimingFinish() const {
        return mCueAimUpTimer <= 0 && mCueAimLeftTimer <= 0 &&
               mCueAimRightTimer <= 0;
    }

private:
    Simulation();
    virtual ~Simulation();

private:
    u32 mSeed;
    u32 mSeedBackup;
    int mFrame;

    int mCueAimUpTimer;
    int mCueAimLeftTimer;
    int mCueAimRightTimer;

    int mCueAimUp;
    int mCueAimLeft;
    int mCueAimRight;
    EGG::Vector2f mCuePos;
    f32 mCuePower;

    BreakInfo* mpBestBreak;
    bool mIsReplay;
};

} // namespace bah

#endif