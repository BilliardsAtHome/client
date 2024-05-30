#include "Simulation.h"

#include <Pack/RPParty.h>
#include <Pack/RPUtility.h>
#include <cmath>
#include <libkiwi.h>

#define CUE_TURN_SPEED_X 0.0015707965f // pi/2000
#define CUE_TURN_SPEED_Y 0.0062831859f // pi/500

namespace BAH {
namespace {

/**
 * @brief Count number of pocketed balls
 */
u32 GetNumSunk() {
    u32 num = 0;

    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* ball = RP_GET_INSTANCE(RPBilBallManager)->GetBall(i);
        ASSERT(ball != NULL);

        // Ignore cue ball
        if (!ball->IsCueBall() && ball->IsState(RPBilBall::EState_Pocket)) {
            num++;
        }
    }

    return num;
}

/**
 * @brief Count number of balls shot off of the table
 */
u32 GetNumOff() {
    u32 num = 0;

    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* ball = RP_GET_INSTANCE(RPBilBallManager)->GetBall(i);
        ASSERT(ball != NULL);

        // Ignore cue ball
        if (!ball->IsCueBall() && ball->IsState(RPBilBall::EState_OffTable)) {
            num++;
        }
    }

    return num;
}

/**
 * @brief Check whether the break shot fouled
 */
bool GetIsFoul() {
    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* ball = RP_GET_INSTANCE(RPBilBallManager)->GetBall(i);
        ASSERT(ball != NULL);

        // Cue ball pocketed?
        if (ball->IsCueBall() && ball->IsState(RPBilBall::EState_Pocket)) {
            ASSERT(i == 0);
            return true;
        }

        // Any ball shot off the table?
        if (ball->IsState(RPBilBall::EState_OffTable)) {
            return true;
        }
    }

    return false;
}

} // namespace

K_DYNAMIC_SINGLETON_IMPL(Simulation);

/**
 * @brief Constructor
 */
Simulation::Simulation()
    : mTimerUp(0),
      mTimerLeft(0),
      mTimerRight(0),
      mpCurrBreak(NULL),
      mpBestBreak(NULL),
      mIsFirstRun(true),
      mIsFirstTick(false),
      mIsReplay(false),
      mIsFinished(false) {
    mpCurrBreak = new (32) BreakInfo();
    mpBestBreak = new (32) BreakInfo();
    ASSERT(mpCurrBreak != NULL);
    ASSERT(mpBestBreak != NULL);

    // Load previous session information
    LoadUniqueId();
    LoadBestBreak();

    // Default to max power
    mpCurrBreak->power = 150.0f;
}

/**
 * @brief Destructor
 */
Simulation::~Simulation() {
    delete mpCurrBreak;
    delete mpBestBreak;
}

/**
 * @brief Load user unique ID (from DVD or NAND)
 */
void Simulation::LoadUniqueId() {
    // Try to open unique ID from the DVD (file placed by user)
    {
        kiwi::MemStream strm =
            kiwi::FileRipper::Open("user.txt", kiwi::EStorage_DVD);

        if (strm.IsOpen()) {
            mUniqueId = ksl::strtoul(strm.Read_string());

            K_LOG_EX("User from DVD: %u\n", *mUniqueId);
            return;
        }
    }

    // Maybe it's instead on the NAND (saved from entry screen)
    {
        kiwi::MemStream strm =
            kiwi::FileRipper::Open("user.bin", kiwi::EStorage_NAND);

        if (strm.IsOpen()) {
            mUniqueId = strm.Read_u32();

            K_LOG_EX("User from NAND: %u\n", *mUniqueId);
            return;
        }
    }
}

/**
 * @brief Load best break from NAND
 */
void Simulation::LoadBestBreak() {
    ASSERT(mpBestBreak != NULL);

    kiwi::MemStream strm =
        kiwi::FileRipper::Open("best.brk", kiwi::EStorage_NAND);

    if (strm.IsOpen()) {
        mpBestBreak->Read(strm);
    } else {
        // Dummy record will instantly be broken
        mpBestBreak->frame = ULONG_MAX;
    }
}

/**
 * @brief Scene reset (before) callback
 */
void Simulation::BeforeReset() {
    mIsFinished = false;
    mIsFirstTick = true;

    if (mIsReplay) {
        // Restore seed for replay
        RPUtlRandom::setSeed(mpBestBreak->seed);
    } else {
        // Record starting seed
        mpCurrBreak->seed = RPUtlRandom::getSeed();
    }
}

/**
 * @brief Scene reset (after) callback
 */
void Simulation::AfterReset() {
    // Don't randomize replay simulation
    if (mIsReplay) {
        mTimerUp = mpBestBreak->up;
        mTimerLeft = mpBestBreak->left;
        mTimerRight = mpBestBreak->right;
        return;
    }

    // Seeded by OS clock
    kiwi::Random random;
    mpCurrBreak->kseed = random.GetSeed();
    mpCurrBreak->frame = 0;

    mTimerUp = mpCurrBreak->up = 0;
    mTimerLeft = mpCurrBreak->left = 0;
    mTimerRight = mpCurrBreak->right = 0;

    // Pick a random style
    EStyle style = static_cast<EStyle>(random.NextU32(EStyle_Max));

    switch (style) {
    case EStyle_Normal:
        // 50% chance to aim up
        if (random.Chance(0.5f)) {
            // Randomize aiming UP frames -> [0f, 35f]
            mTimerUp = mpCurrBreak->up = random.NextU32(35);
        }

        // 80% chance to aim sideways
        if (random.Chance(0.8f)) {
            // 50% chance to aim left vs. aim right
            if (random.Chance(0.5f)) {
                // Randomize aiming SIDEWAYS frames -> [0f, 12f]
                mTimerLeft = mpCurrBreak->left = random.NextU32(12);
            } else {
                // Randomize aiming SIDEWAYS frames -> [0f, 12f]
                mTimerRight = mpCurrBreak->right = random.NextU32(12);
            }
        }

        // Base cue position
        mpCurrBreak->pos = EGG::Vector2f(0.015f, 0.15f);

        // Randomize X pos -> [-0.015, +0.015]
        mpCurrBreak->pos.x *= random.NextF32();
        // 50% chance to flip
        mpCurrBreak->pos.x *= random.Sign();

        // Randomize Y pos -> [+0.15, +0.30]
        mpCurrBreak->pos.y += random.NextF32(0.15f);
        break;

    case EStyle_Jump:
        // Randomize aiming UP frames -> [40f, 55f]
        mTimerUp = mpCurrBreak->up = random.NextU32(40, 55);

        // 50% chance to aim sideways
        if (random.Chance(0.5f)) {
            // 50% chance to aim left vs. aim right
            if (random.Chance(0.5f)) {
                // Randomize aiming SIDEWAYS frames -> [0f, 8f]
                mTimerLeft = mpCurrBreak->left = random.NextU32(8);
            } else {
                // Randomize aiming SIDEWAYS frames -> [0f, 8f]
                mTimerRight = mpCurrBreak->right = random.NextU32(8);
            }
        }

        // Base cue position
        mpCurrBreak->pos = EGG::Vector2f(0.015f, 0.15f);

        // Randomize X pos -> [-0.015, +0.015]
        mpCurrBreak->pos.x *= random.NextF32();
        // 50% chance to flip
        mpCurrBreak->pos.x *= random.Sign();

        // Randomize Y pos -> [+0.15, +0.35]
        mpCurrBreak->pos.y += random.NextF32(0.20f);
        break;
    }
}

/**
 * @brief Run simulation tick
 */
void Simulation::Tick() {
    mpCurrBreak->frame++;

    // For some reason, CanCtrl is wrong on the very first scene tick
    RPBilCtrl* ctrl = RP_GET_INSTANCE(RPBilCtrlManager)->GetCtrl();
    if (ctrl->CanCtrl() && !mIsFirstTick) {
        // Aim up
        if (mTimerUp > 0) {
            ctrl->TurnY(-CUE_TURN_SPEED_Y);
            mTimerUp--;
        }

        // Aim left
        if (mTimerLeft > 0) {
            ctrl->TurnX(CUE_TURN_SPEED_X);
            mTimerLeft--;
        }
        // Aim right
        else if (mTimerRight > 0) {
            ctrl->TurnX(-CUE_TURN_SPEED_X);
            mTimerRight--;
        }
    }

    // Controller IR coordinates
    f32 x = mIsReplay ? mpBestBreak->pos.x : mpCurrBreak->pos.x;
    f32 y = mIsReplay ? mpBestBreak->pos.y : mpCurrBreak->pos.y;

    // Map to screen position
    EGG::Vector2f aim(x * (EGG::Screen::GetSizeXMax() / 2),
                      -y * (EGG::Screen::GetSizeYMax() / 2));

    // Update cue cursor
    RPBilCue* cue = RP_GET_INSTANCE(RPBilCueManager)->GetCue(0);
    ASSERT(cue != NULL);
    cue->SetAimPosition(aim);

    mIsFirstTick = false;
}

/**
 * @brief End of break shot
 */
void Simulation::Finish() {
    mIsFirstRun = false;
    mIsFinished = true;

    // Record break results
    if (!mIsReplay) {
        mpCurrBreak->sunk = GetNumSunk();
        mpCurrBreak->off = GetNumOff();
        mpCurrBreak->foul = GetIsFoul();

        // Upload 6+ breaks to submission server
        if (mpCurrBreak->sunk + mpCurrBreak->off >= 6) {
            mpCurrBreak->Upload();
        }

        // Check for new local best
        if (mpCurrBreak->IsBetterThan(*mpBestBreak)) {
            // Record break locally
            mpCurrBreak->Log();
            mpCurrBreak->Save("best.brk");

            // Prepare replay
            *mpBestBreak = *mpCurrBreak;
            mIsReplay = true;
        }
    }
    // End replay
    else {
        mIsReplay = false;
    }
}

} // namespace BAH