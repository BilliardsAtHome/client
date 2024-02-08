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
 * @brief Check whether the current shot fouled
 */
bool GetIsFoul() {
    RPBilBallManager* m = RPBilBallManager::GetInstance();
    ASSERT(m != NULL);

    // Cue ball pocketed?
    RPBilBall* cueBall = m->GetBall(0);
    ASSERT(cueBall != NULL && cueBall->IsCueBall());
    if (cueBall->IsState(RPBilBall::EState_Pocket)) {
        return true;
    }

    // Ball shot off the table?
    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* ball = m->GetBall(i);
        ASSERT(ball != NULL);

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
Simulation::BreakInfo::BreakInfo()
    : seed(0),
      kseed(0),
      num(0),
      frame(0),
      up(0),
      left(0),
      right(0),
      pos(),
      power(0.0f),
      foul(false) {}

/**
 * @brief Deserialize from stream
 */
void Simulation::BreakInfo::Read(kiwi::IStream& strm) {
    seed = strm.Read_u32();
    kseed = strm.Read_u32();
    num = strm.Read_u32();
    frame = strm.Read_u32();
    up = strm.Read_s32();
    left = strm.Read_s32();
    right = strm.Read_s32();
    pos.x = strm.Read_f32();
    pos.y = strm.Read_f32();
    power = strm.Read_f32();
    foul = strm.Read_bool();

    // Checksum for integrity
    kiwi::Checksum crc;
    crc.Process(this, sizeof(BreakInfo));

    u32 expected = crc.Result();
    u32 got = strm.Read_u32();
    K_WARN_EX(expected != got, "Checksum mismatch (expected %08X, got %08X)",
              expected, got);
}

/**
 * @brief Serialize to stream
 */
void Simulation::BreakInfo::Write(kiwi::IStream& strm) {
    // Checksum for integrity
    kiwi::Checksum crc;
    crc.Process(this, sizeof(BreakInfo));

    strm.Write_u32(seed);
    strm.Write_u32(kseed);
    strm.Write_u32(num);
    strm.Write_u32(frame);
    strm.Write_s32(up);
    strm.Write_s32(left);
    strm.Write_s32(right);
    strm.Write_f32(pos.x);
    strm.Write_f32(pos.y);
    strm.Write_f32(power);
    strm.Write_bool(foul);
    strm.Write_u32(crc.Result());
}

/**
 * @brief Constructor
 */
Simulation::Simulation() : kiwi::ISceneHook(RPSysSceneCreator::RP_BIL_SCENE) {}

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
    if (mpBreakInfo == NULL) {
        mpBreakInfo = new (32) BreakInfo();
    }

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
#pragma unused(scene)

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
        kiwi::CtrlMgr::GetInstance().GetWiiCtrl(kiwi::EPlayer_1);

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
    ASSERT(name != NULL);

    kiwi::NandStream strm(name, kiwi::EOpenMode_Write, true);
    ASSERT(strm.IsOpen());

    mpBreakInfo->Write(strm);
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
    Save("last.brk");

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
        LOG("BREAK = {");
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
}

} // namespace bah