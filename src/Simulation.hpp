#ifndef BAH_CLIENT_SIMULATION_H
#define BAH_CLIENT_SIMULATION_H
#include <libkiwi.h>
#include <types.h>

namespace bah {

/**
 * @brief Billiards simulation runner
 */
class Simulation : public kiwi::DynamicSingleton<Simulation>,
                   public kiwi::ISceneHook {
    friend class kiwi::DynamicSingleton<Simulation>;

public:
    struct BreakInfo {
        BreakInfo();
        void Read(kiwi::IStream& strm);
        void Write(kiwi::IStream& strm) const;

        bool IsBetterThan(const BreakInfo& other) const;
        void Log() const;
        void Save(const char* name) const;

        u32 seed;
        u32 kseed;

        u32 sunk;
        u32 off;
        u32 frame;

        int up;
        int left;
        int right;

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
    int mTimerUp;
    int mTimerLeft;
    int mTimerRight;
    BreakInfo* mpCurrBreak;
    BreakInfo* mpBestBreak;
    bool mIsReplay;
};

} // namespace bah

#endif