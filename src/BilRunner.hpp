#ifndef BIL_RUNNER_H
#define BIL_RUNNER_H
#include <libkiwi.h>
#include <types.h>

/**
 * @brief Billiards simulation runner
 */
class BilRunner : public kiwi::DynamicSingleton<BilRunner>,
                  public kiwi::IScnHook {
    friend class kiwi::DynamicSingleton<BilRunner>;

public:
    virtual void Configure(RPSysScene* scene);
    virtual void BeforeReset(RPSysScene* scene);
    virtual void AfterReset(RPSysScene* scene);

    void Simulate();
    void OnEndShot();

    f32 GetCuePower() const {
        return mIsReplay ? mBestBreak.power : mCuePower;
    }

    bool IsReplay() const {
        return mIsReplay;
    }

    bool IsAimingFinish() const {
        return mCueAimUpTimer <= 0 && mCueAimLeftTimer <= 0 &&
               mCueAimRightTimer <= 0;
    }

private:
    BilRunner();
    virtual ~BilRunner();

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
    } mBestBreak;

    BreakInfo* mpBreakWork;

    bool mIsReplay;
};

#endif