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
    : kiwi::ISceneHook(kiwi::ESceneID_RPBilScene),
      mTimerUp(0),
      mTimerLeft(0),
      mTimerRight(0),
      mpCurrBreak(NULL),
      mpBestBreak(NULL),
      mIsFirstRun(true),
      mIsFirstTick(false),
      mIsReplay(false),
      mIsFinished(false),
      mNumBreak(0) {
    std::memset(mNumBall, 0, sizeof(mNumBall));

    mpCurrBreak = new (32) BreakInfo();
    ASSERT(mpCurrBreak != NULL);

    mpBestBreak = new (32) BreakInfo();
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
 * @brief Configure callback
 */
void Simulation::Configure(RPSysScene* scene) {
#pragma unused(scene)

    RP_GET_INSTANCE(RPGrpRenderer)->AppendDrawObject(this);
}

/**
 * @brief User-level draw (break statistics)
 */
void Simulation::UserDraw() {
    if (IsReplay()) {
        return;
    }

    /**
     * Best break statistics
     */

    kiwi::DebugPrint::PrintfOutline(0.2f, 0.7f, 0.8f, true, kiwi::Color::CYAN,
                                    kiwi::Color::BLACK, "[Best break]");

    kiwi::DebugPrint::PrintfOutline(
        0.2f, 0.6f, 0.8f, true, kiwi::Color::WHITE, kiwi::Color::BLACK,
        "> %d balls (%d sunk, %d off)", mpBestBreak->sunk + mpBestBreak->off,
        mpBestBreak->sunk, mpBestBreak->off);

    kiwi::DebugPrint::PrintfOutline(
        0.2f, 0.5f, 0.8f, true, kiwi::Color::WHITE, kiwi::Color::BLACK,
        "> in %03d frames (%.2f sec)", mpBestBreak->frame,
        mpBestBreak->frame / 60.0f);

    kiwi::DebugPrint::PrintfOutline(
        0.2f, 0.4f, 0.8f, true, kiwi::Color::WHITE, kiwi::Color::BLACK,
        "> %02df up, %02df left, %02df right", mpBestBreak->up,
        mpBestBreak->left, mpBestBreak->right);

    kiwi::DebugPrint::PrintfOutline(0.2f, 0.3f, 0.8f, true, kiwi::Color::YELLOW,
                                    kiwi::Color::BLACK, "> %s",
                                    mpBestBreak->foul ? "foul" : "no foul");

    /**
     * Session statistics
     */

    kiwi::DebugPrint::PrintfOutline(0.2f, -0.3f, 0.8f, true, kiwi::Color::CYAN,
                                    kiwi::Color::BLACK, "[This session]");

    kiwi::DebugPrint::PrintfOutline(0.2f, -0.4f, 0.8f, true, kiwi::Color::WHITE,
                                    kiwi::Color::BLACK, "> %d total breaks",
                                    mNumBreak);

    kiwi::DebugPrint::PrintfOutline(0.2f, -0.5f, 0.8f, true, kiwi::Color::WHITE,
                                    kiwi::Color::BLACK, "> distribution:");

    kiwi::DebugPrint::PrintfOutline(0.2f, -0.6f, 0.8f, true, kiwi::Color::WHITE,
                                    kiwi::Color::BLACK, "{%d, %d, %d, %d, %d}",
                                    mNumBall[0], mNumBall[1], mNumBall[2],
                                    mNumBall[3], mNumBall[4]);
    kiwi::DebugPrint::PrintfOutline(0.2f, -0.7f, 0.8f, true, kiwi::Color::WHITE,
                                    kiwi::Color::BLACK, "{%d, %d, %d, %d, %d}",
                                    mNumBall[5], mNumBall[6], mNumBall[7],
                                    mNumBall[8], mNumBall[9]);

    /**
     * User information
     */

    kiwi::DebugPrint::PrintfOutline(-0.5f, -0.9f, 0.8f, true, kiwi::Color::RED,
                                    kiwi::Color::BLACK, "Unique ID: %06d",
                                    *mUniqueId);
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
        if (random.CoinFlip()) {
            // Randomize aiming UP frames -> [0f, 35f]
            mTimerUp = mpCurrBreak->up = random.NextU32(35);
        }

        // 80% chance to aim sideways
        if (random.Chance(0.8f)) {
            // 50% chance to aim left vs. aim right
            if (random.CoinFlip()) {
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
        if (random.CoinFlip()) {
            // 50% chance to aim left vs. aim right
            if (random.CoinFlip()) {
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

        // Track statistics
        mNumBreak++;
        mNumBall[mpCurrBreak->sunk]++;

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