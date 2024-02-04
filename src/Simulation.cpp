#include "Simulation.hpp"

#include <Pack/RPParty.h>
#include <Pack/RPUtility.h>
#include <cmath>
#include <libkiwi.h>

#define CUE_TURN_SPEED_X 0.0015707965f // pi/2000
#define CUE_TURN_SPEED_Y 0.0062831859f // pi/500

namespace bah {
namespace {

/**
 * @brief Count number of pocketed balls
 */
u32 GetNumPocket() {
    RPBilBallManager* m = RPBilBallManager::GetInstance();
    ASSERT(m != NULL);

    u32 num = 0;

    for (int i = 0; i < RPBilBallManager::scBallNum; i++) {
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
 * @brief Check whether the current shot fouled
 */
bool GetIsFoul() {
    RPBilBallManager* m = RPBilBallManager::GetInstance();
    ASSERT(m != NULL);

    RPBilBall* cueBall = m->GetBall(0);
    ASSERT(cueBall != NULL);
    ASSERT(cueBall->IsCueBall());

    // Only way to foul on break is by pocketing the cue ball
    return cueBall->IsState(RPBilBall::EState_Pocket);
}

} // namespace

K_DYNAMIC_SINGLETON_IMPL(Simulation);

/**
 * @brief Constructor
 */
Simulation::Simulation() : kiwi::IScnHook(RPSysSceneCreator::RP_BIL_SCENE) {}

/**
 * @brief Destructor
 */
Simulation::~Simulation() {
    delete mpBreakInfo;
    mpBreakInfo = NULL;
}

/**
 * @brief Scene configure callback
 */
void Simulation::Configure(RPSysScene* scene) {
#pragma unused(scene)
    mpBreakInfo = new (32) BreakInfo();
    ASSERT(mpBreakInfo != NULL);

    mIsReplay = false;

    // TODO: Default to max power. Maybe configurable later?
    mpBreakInfo->power = 150.0f;

    // Dummy record will instantly be broken
    mBestNum = 0;
    mBestFrame = INT_MAX;
}

/**
 * @brief Scene reset (before) callback
 */
void Simulation::BeforeReset(RPSysScene* scene) {
    // Reuse seed for replay
    if (mIsReplay) {
        RPUtlRandom::setSeed(mpBreakInfo->seed);
        return;
    }

    // Record next seed
    mpBreakInfo->seed = RPUtlRandom::getSeed();
}

/**
 * @brief Scene reset (after) callback
 */
void Simulation::AfterReset(RPSysScene* scene) {
#pragma unused(scene)

    // Just reload what we need to replay the shot
    if (mIsReplay) {
        mTimerUp = mpBreakInfo->up;
        mTimerLeft = mpBreakInfo->left;
        mTimerRight = mpBreakInfo->right;
        return;
    }

    // Seeded by OS clock
    kiwi::Random random;

    mpBreakInfo->frame = 0;
    mTimerUp = mpBreakInfo->up = 0;
    mTimerLeft = mpBreakInfo->left = 0;
    mTimerRight = mpBreakInfo->right = 0;

    // 50% chance to aim up, 50% chance to aim sideways
    if (random.Chance(0.5f)) {
        // Randomize aiming UP frames -> [0f, 35f]
        mTimerUp = mpBreakInfo->up = random.NextU32(35);
    } else {
        // Randomize aiming SIDEWAYS frames -> [0f, 12f]
        // 50% chance to aim left vs. aim right
        if (random.Chance(0.5f)) {
            mTimerLeft = mpBreakInfo->left = random.NextU32(12);
        } else {
            mTimerRight = mpBreakInfo->right = random.NextU32(12);
        }
    }

    // Base cue position
    mpBreakInfo->pos = EGG::Vector2f(0.015f, 0.15f);

    // Randomize X pos -> [-0.015, +0.015]
    mpBreakInfo->pos.x *= random.NextF32();
    // 50% chance to flip
    mpBreakInfo->pos.x *= random.Sign();

    // Randomize Y pos -> [+0.15, +0.30]
    mpBreakInfo->pos.y += random.NextF32(0.15f);
}

/**
 * @brief Run simulation tick
 */
void Simulation::Tick() {
    mpBreakInfo->frame++;

    RPBilCtrl* cueCtrl = RPBilCtrlManager::GetInstance()->GetCtrl();
    if (cueCtrl->CanCtrl()) {
        // Aim up
        if (mTimerUp > 0) {
            cueCtrl->TurnY(-CUE_TURN_SPEED_Y);
            mTimerUp--;
        }
        // Aim left
        else if (mTimerLeft > 0) {
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
        kiwi::CtrlMgr::GetInstance().GetWiiCtrl(kiwi::Player_1);

    // Override IR position
    if (wiiCtrl.Connected()) {
        wiiCtrl.Raw().pos.x = mpBreakInfo->pos.x;
        wiiCtrl.Raw().pos.y = mpBreakInfo->pos.y;
    }
}

/**
 * @brief Save current break info to the NAND
 *
 * @param name File name
 */
void Simulation::Save(const char* name) {
    NANDFileInfo info;
    s32 result;

    result = NANDCreate(name, NAND_PERM_RWALL, 0);
    ASSERT_EX(result == NAND_RESULT_OK || result == NAND_RESULT_EXISTS,
              "NANDCreate failed (%d)", result);

    result = NANDOpen(name, &info, NAND_ACCESS_WRITE);
    ASSERT_EX(result == NAND_RESULT_OK, "NANDOpen failed (%d)", result);

    result = NANDWrite(&info, mpBreakInfo, sizeof(BreakInfo));
    ASSERT_EX(result == sizeof(BreakInfo), "NANDWrite failed (%d)", result);

    result = NANDClose(&info);
    ASSERT_EX(result == NAND_RESULT_OK, "NANDClose failed (%d)", result);
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
    mpBreakInfo->num = GetNumPocket();
    mpBreakInfo->foul = GetIsFoul();

    // Best ball count (or 7+)
    mIsReplay = (mpBreakInfo->num > mBestNum) || (mpBreakInfo->num >= 7);
    // Tied ball count, best frame count
    mIsReplay |=
        (mpBreakInfo->num == mBestNum) && (mpBreakInfo->frame < mBestFrame);

    // Record best shot
    if (mIsReplay) {
        mBestNum = mpBreakInfo->num;
        mBestFrame = mpBreakInfo->frame;

        // clang-format off
        LOG("brk = {");
        LOG_EX("    seed:\t%08X",  mpBreakInfo->seed);
        LOG_EX("    num:\t%d",     mpBreakInfo->num);
        LOG_EX("    frame:\t%d",   mpBreakInfo->frame);
        LOG_EX("    up:\t%d",      mpBreakInfo->up);
        LOG_EX("    left:\t%d",    mpBreakInfo->left);
        LOG_EX("    right:\t%d",   mpBreakInfo->right);
        LOG_EX("    pos:\t{%08X, %08X}",
                                   *(u32*)&mpBreakInfo->pos.x, *(u32*)&mpBreakInfo->pos.y);
        LOG_EX("    power:\t%.2f", mpBreakInfo->power);
        LOG_EX("    foul:\t%s",    mpBreakInfo->foul ? "true" : "false");
        LOG("}");
        // clang-format on

        Save("best.brk");
    }

    // Save every break
    Save("last.brk");
}

} // namespace bah