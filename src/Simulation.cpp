#include "Simulation.hpp"

#include <Pack/RPParty.h>
#include <Pack/RPUtility.h>

/**
 * I don't know what these values represent, but these are exactly what the game
 * uses.
 */
#define CUE_AIM_X_AMT (0.0015707965f)
#define CUE_AIM_Y_AMT (0.0062831859f)

namespace bah {
namespace {

/**
 * @brief Count number of pocketed balls
 */
int GetNumPocket() {
    RPBilBallManager* m = RPBilBallManager::GetInstance();
    ASSERT(m != NULL);

    int num = 0;

    for (int i = 0; i < RPBilBallManager::scBallNum; i++) {
        RPBilBall* ball = m->GetBall(i);
        ASSERT(ball != NULL);

        // Ignore cue ball
        if (ball->IsCueBall()) {
            continue;
        }

        // Check for POCKET state
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
    delete mpBestBreak;
    mpBestBreak = NULL;
}

/**
 * @brief Scene configure callback
 */
void Simulation::Configure(RPSysScene* scene) {
#pragma unused(scene)
    // Need 32 align because NAND is cool like that
    mpBestBreak = new (32) BreakInfo();
    ASSERT(mpBestBreak != NULL);

    mSeed = 0;
    mSeedBackup = 0;

    mFrame = -1;
    mIsReplay = false;

    mCuePower = 150.0f;

    mpBestBreak->seed = 0xFFFFFFFF;
    mpBestBreak->num = 0;
    mpBestBreak->frame = 0xFFFFFFFF;
    mpBestBreak->pos = EGG::Vector2f(0.0f, 0.0f);
    mpBestBreak->power = 0.0f;
    mpBestBreak->foul = true;
}

/**
 * @brief Scene reset (before) callback
 */
void Simulation::BeforeReset(RPSysScene* scene) {
    // Next RNG
    mSeed = RPUtlRandom::getSeed();

    // Replay RNG
    if (mIsReplay) {
        // Backup path for later.
        // Continuing after replay means we would follow the same path.
        mSeedBackup = mSeed;
        RPUtlRandom::setSeed(mpBestBreak->seed);
    }
}

/**
 * @brief Scene reset (after) callback
 */
void Simulation::AfterReset(RPSysScene* scene) {
#pragma unused(scene)
    mFrame = -1;

    // Backup RNG
    u32 seed = RPUtlRandom::getSeed();

    // Reset timers
    mCueAimUp = mCueAimUpTimer = 0;
    mCueAimLeft = mCueAimLeftTimer = 0;
    mCueAimRight = mCueAimRightTimer = 0;

    // 50% chance to aim up, 50% chance to aim sideways
    if (RPUtlRandom::getF32() > 0.5f) {
        // Randomize aiming UP frames -> [0f, 35f]
        mCueAimUp = RPUtlRandom::getU32(0, 35);
        mCueAimUpTimer = mCueAimUp;
    } else {
        // Randomize aiming SIDEWAYS frames -> [0f, 12f]
        // 50% chance to aim left vs. aim right
        if (RPUtlRandom::getF32() > 0.5f) {
            mCueAimLeft = RPUtlRandom::getU32(0, 12);
            mCueAimLeftTimer = mCueAimLeft;
        } else {
            mCueAimRight = RPUtlRandom::getU32(0, 12);
            mCueAimRightTimer = mCueAimRight;
        }
    }

    // Base cue position
    mCuePos = EGG::Vector2f(0.015f, 0.15f);

    // Randomize X pos -> [-0.015, +0.015]
    mCuePos.x = RPUtlRandom::getF32() * mCuePos.x;
    // 50% chance to flip
    mCuePos.x *= RPUtlRandom::getF32() > 0.5f ? 1.0f : -1.0f;

    // Randomize Y pos -> [+0.15, +0.30]
    mCuePos.y += RPUtlRandom::getF32() * 0.15f;

    // Restore RNG
    RPUtlRandom::setSeed(seed);
}

/**
 * @brief Run simulation tick
 */
void Simulation::Tick() {
    // Increment frame count (-1 means begin now)
    if (mFrame == -1) {
        mFrame = 0;
    } else {
        mFrame++;
    }

    // Aim cue
    RPBilCtrl* cueCtrl = RPBilCtrlManager::GetInstance()->GetCtrl();
    if (cueCtrl->CanCtrl()) {
        // Aim up
        if (mCueAimUpTimer > 0) {
            cueCtrl->TurnY(-CUE_AIM_Y_AMT);
            mCueAimUpTimer--;
        }
        // Aim left
        else if (mCueAimLeftTimer > 0) {
            cueCtrl->TurnX(CUE_AIM_X_AMT);
            mCueAimLeftTimer--;
        }
        // Aim right
        else if (mCueAimRightTimer > 0) {
            cueCtrl->TurnX(-CUE_AIM_X_AMT);
            mCueAimRightTimer--;
        }
    }

    // Access P1 remote
    kiwi::WiiCtrl& wiiCtrl =
        kiwi::CtrlMgr::GetInstance().GetWiiCtrl(kiwi::Player_1);

    // Override IR position
    if (wiiCtrl.Connected()) {
        wiiCtrl.Raw().pos.x = mIsReplay ? mpBestBreak->pos.x : mCuePos.x;
        wiiCtrl.Raw().pos.y = mIsReplay ? mpBestBreak->pos.y : mCuePos.y;
    }
}

/**
 * @brief End-of-shot callback
 */
void Simulation::OnEndShot() {
    // End replay
    if (mIsReplay) {
        mIsReplay = false;

        // Restore backup
        RPUtlRandom::setSeed(mSeedBackup);
        return;
    }

    // Check for new best break
    int num = GetNumPocket();
    bool isFoul = GetIsFoul();
    bool best = false;

    // Log 4 or better breaks for fun
    if (num >= 4) {
        LOG_EX("Notable break: %d pocketed in %df", num, mFrame);
    }

    // Prioritize bigger breaks over anything else
    if (num > mpBestBreak->num) {
        best = true;
    }

    // Break ties by frame count
    if ((num == mpBestBreak->num) && (mFrame < mpBestBreak->frame)) {
        best = true;
    }

    // Break ties by non-foul
    if ((num == mpBestBreak->num) && (mFrame == mpBestBreak->frame) &&
        (mpBestBreak->foul && !isFoul)) {
        best = true;
    }

    if (best) {
        /**
         * Save best break info
         */
        mpBestBreak->seed = mSeed;
        mpBestBreak->num = num;
        mpBestBreak->frame = mFrame;
        mpBestBreak->aimU = mCueAimUp;
        mpBestBreak->aimL = mCueAimLeft;
        mpBestBreak->aimR = mCueAimRight;
        mpBestBreak->pos = mCuePos;
        mpBestBreak->power = mCuePower;
        mpBestBreak->foul = isFoul;

        /**
         * Dump to console
         */
        // clang-format off
        LOG("[New best break!]");
        LOG_EX("    Seed:\t%08X", mpBestBreak->seed);
        LOG_EX("    Foul:\t%s",   mpBestBreak->foul ? "true" : "false");
        LOG_EX("    Num:\t%d",    mpBestBreak->num);
        LOG_EX("    Frame:\t%d",  mpBestBreak->frame);

        LOG   ("    [Aim] (-1f):");
        LOG_EX("        Up:\t%d",    mpBestBreak->aimU);
        LOG_EX("        Left:\t%d",  mpBestBreak->aimL);
        LOG_EX("        Right:\t%d", mpBestBreak->aimR);

        LOG_EX("    Pos:\t{X=%.2f (%08X), Y=%.2f (%08X)}",
            mpBestBreak->pos.x, *(u32*)&mpBestBreak->pos.x, mpBestBreak->pos.y, *(u32*)&mpBestBreak->pos.y);
        
        LOG_EX("    Power:\t%.2f (%08X)",
            mpBestBreak->power, *(u32*)&mpBestBreak->power);
        // clang-format on

        /**
         * Write to NAND
         */
        NANDFileInfo info;
        s32 result;

        // Create NAND file
        result = NANDCreate("best.brk", NAND_PERM_RWALL, 0);
        ASSERT(result == NAND_RESULT_OK || result == NAND_RESULT_EXISTS);

        // Open NAND file
        result = NANDOpen("best.brk", &info, NAND_ACCESS_WRITE);
        ASSERT(result == NAND_RESULT_OK);

        // Write + commit NAND file
        result = NANDWrite(&info, mpBestBreak, sizeof(BreakInfo));
        NANDClose(&info);
        ASSERT_EX(result > 0, "NANDWrite failed (%d)", result);

        // Replay new best break
        mIsReplay = true;
    }
}

} // namespace bah