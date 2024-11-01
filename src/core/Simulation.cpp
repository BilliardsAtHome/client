#include "core/Simulation.h"

#include "core/BreakInfo.h"
#include "core/RichPresenceProfile.h"

#include <Pack/RPParty.h>
#include <Pack/RPUtility.h>
#include <cmath>
#include <libkiwi.h>

K_DYNAMIC_SINGLETON_IMPL(BAH::Simulation);

namespace BAH {
namespace {

/**
 * @brief Counts the number of balls sunk/pocketed
 */
u32 GetSunkNum() {
    u32 num = 0;

    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* pBall = RP_GET_INSTANCE(RPBilBallManager)->GetBall(i);
        ASSERT(pBall != nullptr);

        // Ignore cue ball
        if (pBall->IsCueBall()) {
            continue;
        }

        if (pBall->IsState(RPBilBall::EState_Pocket)) {
            num++;
        }
    }

    return num;
}

/**
 * @brief Counts the number of balls shot off of the table
 */
u32 GetOffNum() {
    u32 num = 0;

    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* pBall = RP_GET_INSTANCE(RPBilBallManager)->GetBall(i);
        ASSERT(pBall != nullptr);

        // Ignore cue ball
        if (pBall->IsCueBall()) {
            continue;
        }

        if (pBall->IsState(RPBilBall::EState_OffTable)) {
            num++;
        }
    }

    return num;
}

/**
 * @brief Tests whether the break shot fouled
 */
bool GetFoul() {
    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* pBall = RP_GET_INSTANCE(RPBilBallManager)->GetBall(i);
        ASSERT(pBall != nullptr);

        // Cue ball pocketed?
        if (pBall->IsCueBall() && pBall->IsState(RPBilBall::EState_Pocket)) {
            ASSERT(i == 0);
            return true;
        }

        // Any ball shot off the table?
        if (pBall->IsState(RPBilBall::EState_OffTable)) {
            return true;
        }
    }

    return false;
}

} // namespace

/**
 * @brief Horizontal turn speed
 * @note pi/2000
 */
const f32 Simulation::TURN_SPEED_X = 0.0015707965f;
/**
 * @brief Vertical turn speed
 * @note pi/500
 */
const f32 Simulation::TURN_SPEED_Y = 0.0062831859f;

/**
 * @brief Maximum cue power
 */
const f32 Simulation::POWER_MAX = 150.0f;

/**
 * @brief Constructor
 */
Simulation::Simulation()
    : kiwi::ISceneHook(kiwi::ESceneID_RPBilScene),
      mHttpError(kiwi::EHttpErr_Success),
      mHttpExError(0),
      mHttpStatus(kiwi::EHttpStatus_None),
      mTimerUp(0),
      mTimerLeft(0),
      mTimerRight(0),
      mpCurrBreak(nullptr),
      mpBestBreak(nullptr),
      mIsFirstRun(true),
      mIsFirstTick(false),
      mIsReplay(false),
      mIsFinished(false),
      mBreakNum(0) {

    std::memset(mBreakBallNum, 0, sizeof(mBreakBallNum));

    mpCurrBreak = new (32, kiwi::EMemory_MEM2) BreakInfo();
    ASSERT(mpCurrBreak != nullptr);

    mpBestBreak = new (32, kiwi::EMemory_MEM2) BreakInfo();
    ASSERT(mpBestBreak != nullptr);

    // Load previous session information
    LoadUser();
    LoadBreak();

    // Default to max power
    mpCurrBreak->power = POWER_MAX;
}

/**
 * @brief Destructor
 */
Simulation::~Simulation() {
    delete mpCurrBreak;
    mpCurrBreak = nullptr;

    delete mpBestBreak;
    mpBestBreak = nullptr;
}

/**
 * @brief Configure callback
 *
 * @param pScene Current scene
 */
void Simulation::Configure(RPSysScene* pScene) {
#pragma unused(pScene)

    // Add to renderer for debug display
    RPGrpRenderer::GetCurrent()->AppendDrawObject(this);

    // Start up Discord rich presence
    kiwi::RichPresenceMgr::GetInstance().SetProfile(new RichPresenceProfile());
}

/**
 * @brief Standard draw pass
 */
void Simulation::UserDraw() {
    if (IsReplay()) {
        return;
    }

    /**
     * Best break statistics
     */
    // clang-format off
    kiwi::DebugPrint::PrintfOutline(0.2f, 0.7f, 0.8f, true,
                                    kiwi::Color::CYAN, kiwi::Color::BLACK,
                                    "[Best break]");

    kiwi::DebugPrint::PrintfOutline(0.2f, 0.6f, 0.8f, true,
                                    kiwi::Color::WHITE, kiwi::Color::BLACK,
                                    "> %d balls (%d sunk, %d off)",
                                    mpBestBreak->sunk + mpBestBreak->off,
                                    mpBestBreak->sunk, mpBestBreak->off);

    kiwi::DebugPrint::PrintfOutline(0.2f, 0.5f, 0.8f, true,
                                    kiwi::Color::WHITE, kiwi::Color::BLACK,
                                    "> in %03d frames (%.2f sec)",
                                    mpBestBreak->frame,
                                    mpBestBreak->frame / 60.0f);

    kiwi::DebugPrint::PrintfOutline(0.2f, 0.4f, 0.8f, true,
                                    kiwi::Color::WHITE, kiwi::Color::BLACK,
                                    "> %02df up, %02df left, %02df right",
                                    mpBestBreak->up, mpBestBreak->left,
                                    mpBestBreak->right);

    kiwi::DebugPrint::PrintfOutline(0.2f, 0.3f, 0.8f, true,
                                    kiwi::Color::YELLOW, kiwi::Color::BLACK,
                                    "> %s",
                                    mpBestBreak->foul ? "foul" : "no foul");
    // clang-format on

    /**
     * Session statistics
     */
    // clang-format off
    kiwi::DebugPrint::PrintfOutline(0.2f, -0.3f, 0.8f, true,
                                    kiwi::Color::CYAN, kiwi::Color::BLACK,
                                    "[This session]");

    kiwi::DebugPrint::PrintfOutline(0.2f, -0.4f, 0.8f, true,
                                    kiwi::Color::WHITE, kiwi::Color::BLACK,
                                    "> %d total breaks", mBreakNum);

    kiwi::DebugPrint::PrintfOutline(0.2f, -0.5f, 0.8f, true,
                                    kiwi::Color::WHITE, kiwi::Color::BLACK,
                                    "> distribution:");

    kiwi::DebugPrint::PrintfOutline(0.2f, -0.6f, 0.8f, true,
                                    kiwi::Color::WHITE, kiwi::Color::BLACK,
                                    "{%d, %d, %d, %d, %d}",
                                    mBreakBallNum[0], mBreakBallNum[1],
                                    mBreakBallNum[2], mBreakBallNum[3],
                                    mBreakBallNum[4]);
                                    
    kiwi::DebugPrint::PrintfOutline(0.2f, -0.7f, 0.8f, true,
                                    kiwi::Color::WHITE, kiwi::Color::BLACK,
                                    "{%d, %d, %d, %d, %d}",
                                    mBreakBallNum[5], mBreakBallNum[6],
                                    mBreakBallNum[7], mBreakBallNum[8],
                                    mBreakBallNum[9]);
    // clang-format on

    if (mIsConnected.HasValue()) {
        // clang-format off
        kiwi::DebugPrint::PrintfOutline(-0.5f, -0.8f, 0.8f, true,
                                        *mIsConnected ? kiwi::Color::GREEN : kiwi::Color::YELLOW, kiwi::Color::BLACK,
                                        *mIsConnected ? "Online" : "Offline (err:%d ex:%d stat:%d)",
                                        mHttpError, mHttpExError, mHttpStatus);
        // clang-format on
    }

    // clang-format off
    kiwi::DebugPrint::PrintfOutline(-0.5f, -0.9f, 0.8f, true,
                                    kiwi::Color::RED, kiwi::Color::BLACK,
                                    "Unique ID: %06d", *mUniqueID);
    // clang-format on
}

/**
 * @brief Loads user info (from DVD or NAND)
 */
void Simulation::LoadUser() {
    // Try to open DVD file (file placed by user)
    {
        kiwi::MemStream strm =
            kiwi::FileRipper::Open("user.txt", kiwi::EStorage_DVD);

        if (strm.IsOpen()) {
            mUniqueID = ksl::strtoul(strm.Read_string());
            K_LOG_EX("User from DVD: %u\n", *mUniqueID);
            return;
        }
    }

    // Try to open NAND file (saved by login scene)
    {
        kiwi::MemStream strm =
            kiwi::FileRipper::Open("user.bin", kiwi::EStorage_NAND);

        if (strm.IsOpen()) {
            mUniqueID = strm.Read_u32();
            K_LOG_EX("User from NAND: %u\n", *mUniqueID);
            return;
        }
    }
}

/**
 * @brief Loads break info (from NAND)
 */
void Simulation::LoadBreak() {
    ASSERT(mpBestBreak != nullptr);

    kiwi::MemStream strm =
        kiwi::FileRipper::Open("best.brk", kiwi::EStorage_NAND);

    if (strm.IsOpen()) {
        mpBestBreak->Read(strm);
        return;
    }

    // Dummy record will instantly be broken
    mpBestBreak->sunk = 0;
    mpBestBreak->off = 0;
    mpBestBreak->foul = false;
    mpBestBreak->frame = ULONG_MAX;
}

/**
 * @brief Logic before scene reset
 */
void Simulation::BeforeReset() {
    ASSERT(mpCurrBreak != nullptr);
    ASSERT(mpBestBreak != nullptr);

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
 * @brief Logic after scene reset
 */
void Simulation::AfterReset() {
    ASSERT(mpCurrBreak != nullptr);
    ASSERT(mpBestBreak != nullptr);

    // Replay ignores further randomization
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
    case EStyle_Normal: {
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
    }

    case EStyle_Jump: {
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
}

/**
 * @brief Update logic
 */
void Simulation::Tick() {
    ASSERT(mpCurrBreak != nullptr);
    ASSERT(mpBestBreak != nullptr);

    mpCurrBreak->frame++;

    RPBilCtrl* pCtrl = RP_GET_INSTANCE(RPBilCtrlManager)->GetCtrl();

    // TODO: CanCtrl is wrong on the very first scene tick, why?
    if (pCtrl->CanCtrl() && !mIsFirstTick) {
        // Aim up
        if (mTimerUp > 0) {
            mTimerUp--;
            pCtrl->TurnY(-TURN_SPEED_Y);
        }

        // Aim left
        if (mTimerLeft > 0) {
            mTimerLeft--;
            pCtrl->TurnX(TURN_SPEED_X);
        }
        // Aim right
        else if (mTimerRight > 0) {
            mTimerRight--;
            pCtrl->TurnX(-TURN_SPEED_X);
        }
    }

    // Pointer coordinates
    f32 x = mIsReplay ? mpBestBreak->pos.x : mpCurrBreak->pos.x;
    f32 y = mIsReplay ? mpBestBreak->pos.y : mpCurrBreak->pos.y;

    // Map to screen position
    EGG::Vector2f pos(x * (EGG::Screen::GetSizeXMax() / 2),
                      -y * (EGG::Screen::GetSizeYMax() / 2));

    // Update cue cursor
    RPBilCue* pCue = RP_GET_INSTANCE(RPBilCueManager)->GetCue(0);
    ASSERT(pCue != nullptr);
    pCue->SetAimPosition(pos);

    mIsFirstTick = false;
}

/**
 * @brief Commits break results
 */
void Simulation::Finish() {
    ASSERT(mpCurrBreak != nullptr);
    ASSERT(mpBestBreak != nullptr);

    mIsFirstRun = false;
    mIsFinished = true;

    // Nothing to do if this is a replay
    if (mIsReplay) {
        mIsReplay = false;
        return;
    }

    // Record break results
    mpCurrBreak->sunk = GetSunkNum();
    mpCurrBreak->off = GetOffNum();
    mpCurrBreak->foul = GetFoul();

    // Track statistics
    mBreakNum++;
    mBreakBallNum[mpCurrBreak->sunk + mpCurrBreak->off]++;

    bool upload = false;
    // Always upload 6+ breaks
    upload |= mpCurrBreak->sunk + mpCurrBreak->off >= 6;
    // Upload first break to test connection
    // upload |= !mIsConnected.HasValue();

    // Upload information to the server
    if (upload) {
        mIsConnected =
            mpCurrBreak->Upload(mHttpError, mHttpExError, mHttpStatus);
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

} // namespace BAH
