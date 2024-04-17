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
u32 GetNumPocket() {
    RPBilBallManager* m = RPBilBallManager::GetInstance();
    ASSERT(m != NULL);

    u32 num = 0;

    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* ball = m->GetBall(i);
        ASSERT(ball != NULL);

        // Ignore cue ball
        if (ball->IsCueBall()) {
            continue;
        }

        if (ball->IsState(RPBilBall::EState_Pocket)) {
            num++;
        }
    }

    return num;
}

/**
 * @brief Count number of balls shot off of the table
 */
u32 GetNumOffTable() {
    RPBilBallManager* m = RPBilBallManager::GetInstance();
    ASSERT(m != NULL);

    u32 num = 0;

    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* ball = m->GetBall(i);
        ASSERT(ball != NULL);

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
    RPBilBallManager* m = RPBilBallManager::GetInstance();
    ASSERT(m != NULL);

    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* ball = m->GetBall(i);
        ASSERT(ball != NULL);

        // Cue ball pocketed?
        if (i == 0 && ball->IsState(RPBilBall::EState_Pocket)) {
            ASSERT(ball->IsCueBall());
            return true;
        }

        // Ball shot off the table?
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
    : kiwi::ISceneHook(RPSysSceneCreator::ESceneID_RPBilScene),
      mUserId("000000000000000000") {}

/**
 * @brief Destructor
 */
Simulation::~Simulation() {
    delete mpCurrBreak;
    mpCurrBreak = NULL;

    delete mpBestBreak;
    mpBestBreak = NULL;
}

/**
 * @brief Scene configure callback
 */
void Simulation::Configure(RPSysScene* scene) {
#pragma unused(scene)
    // 32-byte aligned because NAND is cool like that :D
    if (mpCurrBreak == NULL) {
        mpCurrBreak = new (32) BreakInfo();
    }
    if (mpBestBreak == NULL) {
        mpBestBreak = new (32) BreakInfo();
    }

    ASSERT(mpCurrBreak != NULL);
    ASSERT(mpBestBreak != NULL);

    // Previous best may still be on the NAND
    kiwi::NandStream strm("best.brk", kiwi::EOpenMode_Read);
    if (strm.IsOpen()) {
        mpBestBreak->Read(strm);
    } else {
        // Dummy record will instantly be broken
        mpBestBreak->frame = ULONG_MAX;
    }

    // Default to max power
    mpCurrBreak->power = 150.0f;
    mIsReplay = false;
}

/**
 * @brief Scene reset (before) callback
 */
void Simulation::BeforeReset(RPSysScene* scene) {
#pragma unused(scene)
    mIsFirstTick = true;

    // Reuse seed for replay
    if (mIsReplay) {
        RPUtlRandom::setSeed(mpBestBreak->seed);
        return;
    }

    // Record next seed
    mpCurrBreak->seed = RPUtlRandom::getSeed();
}

/**
 * @brief Scene reset (after) callback
 */
void Simulation::AfterReset(RPSysScene* scene) {
#pragma unused(scene)

    // Just reload what we need to replay the shot
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

    // 50% chance to aim up
    if (random.Chance(0.5f)) {
        // Randomize aiming UP frames -> [0f, 35f]
        mTimerUp = mpCurrBreak->up = random.NextU32(35);
    }

    // 80% chance to aim sideways
    if (random.Chance(0.80f)) {
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
}

/**
 * @brief Run simulation tick
 */
void Simulation::Tick() {
    mpCurrBreak->frame++;

    // For some reason, CanCtrl is wrong on the very first scene tick
    RPBilCtrl* cueCtrl = RPBilCtrlManager::GetInstance()->GetCtrl();
    if (cueCtrl->CanCtrl() && !mIsFirstTick) {
        // Aim up
        if (mTimerUp > 0) {
            cueCtrl->TurnY(-CUE_TURN_SPEED_Y);
            mTimerUp--;
        }

        // Aim left
        if (mTimerLeft > 0) {
            cueCtrl->TurnX(CUE_TURN_SPEED_X);
            mTimerLeft--;
        }
        // Aim right
        else if (mTimerRight > 0) {
            cueCtrl->TurnX(-CUE_TURN_SPEED_X);
            mTimerRight--;
        }
    }

    kiwi::WiiCtrl& wiiCtrl =
        kiwi::CtrlMgr::GetInstance().GetWiiCtrl(kiwi::EPlayer_1);

    // Override IR position
    if (wiiCtrl.Connected()) {
        wiiCtrl.Raw().pos.x =
            mIsReplay ? mpBestBreak->pos.x : mpCurrBreak->pos.x;

        wiiCtrl.Raw().pos.y =
            mIsReplay ? mpBestBreak->pos.y : mpCurrBreak->pos.y;
    }

    mIsFirstTick = false;
}

/**
 * @brief End-of-shot callback
 */
void Simulation::OnEndShot() {
    // End replay
    if (mIsReplay) {
        mIsReplay = false;
        return;
    }

    // Record break results
    mpCurrBreak->sunk = GetNumPocket();
    mpCurrBreak->off = GetNumOffTable();
    mpCurrBreak->foul = GetIsFoul();

    // Upload 6+ breaks to submission server
    if (mpCurrBreak->sunk + mpCurrBreak->off > 6) {
        mpCurrBreak->Upload();
    }

    mIsReplay = mpCurrBreak->IsBetterThan(*mpBestBreak);
    if (mIsReplay) {
        // Record best shot
        std::memcpy(mpBestBreak, mpCurrBreak, sizeof(BreakInfo));
        mpBestBreak->Log();

        // Write best break to file
        mpBestBreak->Save("best.brk");
    }
}

} // namespace BAH