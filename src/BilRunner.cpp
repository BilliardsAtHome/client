#include "BilRunner.hpp"

#include <Pack/RPParty.h>
#include <Pack/RPUtility.h>

// ???
#define CUE_AIM_X_AMT (0.0015707965f)
#define CUE_AIM_Y_AMT (0.0062831859f)

K_DYNAMIC_SINGLETON_IMPL(BilRunner);

/**
 * @brief Count number of pocketed balls
 */
static int GetNumPocket() {
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
 * @brief Check for a foul
 */
static int GetIsFoul() {
    RPBilBallManager* m = RPBilBallManager::GetInstance();
    ASSERT(m != NULL);

    RPBilBall* cueBall = m->GetBall(0);
    ASSERT(cueBall != NULL);
    ASSERT(cueBall->IsCueBall());

    return cueBall->IsState(RPBilBall::EState_Pocket);
}

/**
 * Constructor
 */
BilRunner::BilRunner() : kiwi::IScnHook(RPSysSceneCreator::RP_BIL_SCENE) {}

/**
 * @brief Destructor
 */
BilRunner::~BilRunner() {
    delete mpBreakWork;
    mpBreakWork = NULL;
}

/**
 * @brief Scene configure callback
 */
void BilRunner::Configure(RPSysScene* scene) {
#pragma unused(scene)
    // Need 32 align because NAND is cool like that
    mpBreakWork = new (32) BreakInfo();
    ASSERT(mpBreakWork != NULL);

    mSeed = 0;
    mSeedBackup = 0;

    mFrame = -1;
    mIsReplay = false;

    mCuePower = 150.0f;

    mBestBreak.seed = 0xFFFFFFFF;
    mBestBreak.num = 0;
    mBestBreak.frame = 0xFFFFFFFF;
    mBestBreak.pos = EGG::Vector2f(0.0f, 0.0f);
    mBestBreak.power = 0.0f;
    mBestBreak.foul = true;
}

/**
 * @brief Scene reset (before) callback
 */
void BilRunner::BeforeReset(RPSysScene* scene) {
    // Next RNG
    mSeed = RPUtlRandom::getSeed();

    // Replay RNG
    if (mIsReplay) {
        // Backup path for later.
        // Continuing after replay means we would follow the same path.
        mSeedBackup = mSeed;
        RPUtlRandom::setSeed(mBestBreak.seed);
    }
}

/**
 * @brief Scene reset (after) callback
 */
void BilRunner::AfterReset(RPSysScene* scene) {
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
void BilRunner::Simulate() {
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
        wiiCtrl.Raw().pos.x = mIsReplay ? mBestBreak.pos.x : mCuePos.x;
        wiiCtrl.Raw().pos.y = mIsReplay ? mBestBreak.pos.y : mCuePos.y;
    }
}

/**
 * @brief End-of-shot callback
 */
void BilRunner::OnEndShot() {
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
    if (num > mBestBreak.num) {
        best = true;
    }

    // Break ties by frame count
    if ((num == mBestBreak.num) && (mFrame < mBestBreak.frame)) {
        best = true;
    }

    // Break ties by non-foul
    if ((num == mBestBreak.num) && (mFrame == mBestBreak.frame) &&
        (mBestBreak.foul && !isFoul)) {
        best = true;
    }

    if (best) {
        /**
         * Save best break info
         */
        mBestBreak.seed = mSeed;
        mBestBreak.num = num;
        mBestBreak.frame = mFrame;
        mBestBreak.aimU = mCueAimUp;
        mBestBreak.aimL = mCueAimLeft;
        mBestBreak.aimR = mCueAimRight;
        mBestBreak.pos = mCuePos;
        mBestBreak.power = mCuePower;
        mBestBreak.foul = isFoul;

        /**
         * Dump to console
         */
        // clang-format off
        LOG("[New best break!]");
        LOG_EX("    Seed:\t%08X", mBestBreak.seed);
        LOG_EX("    Foul:\t%s",   mBestBreak.foul ? "true" : "false");
        LOG_EX("    Num:\t%d",    mBestBreak.num);
        LOG_EX("    Frame:\t%d",  mBestBreak.frame);

        LOG   ("    [Aim] (-1f):");
        LOG_EX("        Up:\t%d",    mBestBreak.aimU);
        LOG_EX("        Left:\t%d",  mBestBreak.aimL);
        LOG_EX("        Right:\t%d", mBestBreak.aimR);

        LOG_EX("    Pos:\t{X=%.2f (%08X), Y=%.2f (%08X)}",
            mBestBreak.pos.x, *(u32*)&mBestBreak.pos.x, mBestBreak.pos.y, *(u32*)&mBestBreak.pos.y);
        
        LOG_EX("    Power:\t%.2f (%08X)",
            mBestBreak.power, *(u32*)&mBestBreak.power);
        // clang-format on

        /**
         * Write to NAND
         */
        NANDFileInfo info;
        s32 result;

        // Copy to workmem (aligned to 32)
        std::memcpy(mpBreakWork, &mBestBreak, sizeof(BreakInfo));

        // Create NAND file
        result = NANDCreate("best.brk", NAND_PERM_RWALL, 0);
        ASSERT(result == NAND_RESULT_OK || result == NAND_RESULT_EXISTS);

        // Open NAND file
        result = NANDOpen("best.brk", &info, NAND_ACCESS_WRITE);
        ASSERT(result == NAND_RESULT_OK);

        // Write + commit NAND file
        result = NANDWrite(&info, mpBreakWork, sizeof(mBestBreak));
        NANDClose(&info);
        ASSERT_EX(result > 0, "NANDWrite failed (%d)", result);

        // Replay new best break
        mIsReplay = true;
    }
}