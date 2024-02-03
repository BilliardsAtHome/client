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
        /**
         * @brief Constructor
         */
        BreakInfo()
            : seed(0),
              num(0),
              frame(0),
              up(0),
              left(0),
              right(0),
              pos(),
              power(0.0f),
              foul(false) {}

        u32 seed;
        u32 num;
        int frame;
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
    void Save(const char* name);
    void OnEndShot();

    f32 GetCuePower() const {
        return mpBreakInfo->power;
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
    BreakInfo* mpBreakInfo;

    int mBestNum;
    int mBestFrame;
    bool mIsReplay;
};

} // namespace bah

#endif