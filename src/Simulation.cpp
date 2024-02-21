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
 * @brief Count number of balls shot off of the table
 */
u32 GetNumOffTable() {
    RPBilBallManager* m = RPBilBallManager::GetInstance();
    ASSERT(m != NULL);

    u32 num = 0;

    for (int i = 0; i < RPBilBallManager::BALL_MAX; i++) {
        RPBilBall* ball = m->GetBall(i);
        ASSERT(ball != NULL);

        if (ball->IsState(RPBilBall::EState_OffTable)) {
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
Simulation::BreakInfo::BreakInfo()
    : seed(0),
      kseed(0),
      sunk(0),
      off(0),
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
    sunk = strm.Read_u32();
    off = strm.Read_u32();
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
void Simulation::BreakInfo::Write(kiwi::IStream& strm) const {
    // Checksum for integrity
    kiwi::Checksum crc;
    crc.Process(this, sizeof(BreakInfo));

    strm.Write_u32(seed);
    strm.Write_u32(kseed);
    strm.Write_u32(sunk);
    strm.Write_u32(off);
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
 * @brief Compare break results
 *
 * @param other Comparison target
 */
bool Simulation::BreakInfo::IsBetterThan(const BreakInfo& other) const {
    // New best pocketed count
    if (sunk > other.sunk) {
        return true;
    }

    // Same sunk, but more were shot off.
    // (In a speedrun scenario, this is still good)
    if (sunk == other.sunk && off > other.off) {
        return true;
    }

    // Same ball count, but faster frame count
    if (sunk == other.sunk && off == other.off && frame < other.frame) {
        return true;
    }

    return false;
}

/**
 * @brief Log break result to the console
 */
void Simulation::BreakInfo::Log() const {
    // clang-format off
    LOG("BREAK = {");
    LOG_EX("    seed:\t%08X",        seed);
    LOG_EX("    kseed:\t%08X",       kseed);
    LOG_EX("    sunk:\t%d",          sunk);
    LOG_EX("    off:\t%d",           off);
    LOG_EX("    frame:\t%d",         frame);
    LOG_EX("    up:\t%d",            up);
    LOG_EX("    left:\t%d",          left);
    LOG_EX("    right:\t%d",         right);
    LOG_EX("    pos:\t{%08X, %08X}", *(u32*)&pos.x, *(u32*)&pos.y);
    LOG_EX("    power:\t%.2f",       power);
    LOG_EX("    foul:\t%s",          foul ? "true" : "false");
    LOG("}");
    // clang-format on
}

/**
 * @brief Save break result to the NAND

 * @param name File name
 * @return Success
 */
void Simulation::BreakInfo::Save(const char* name) const {
    kiwi::NandStream strm(kiwi::EOpenMode_Write);

    while (true) {
        // Attempt to open file
        bool success = strm.Open(name);
        if (success) {
            break;
        }

        // Failed? Try again in one second
        volatile s64 x = OSGetTime();
        while (OSGetTime() - x < OS_SEC_TO_TICKS(1)) {
            ;
        }
    }

    Write(strm);
}

/**
 * @brief Constructor
 */
Simulation::Simulation()
    : kiwi::ISceneHook(RPSysSceneCreator::ESceneID_RPBilScene) {}

/**
 * @brief Destructor
 */
Simulation::~Simulation() {
    delete mpCurrBreak;
    mpCurrBreak = NULL;
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

    // Default to max power.
    // TODO: Maybe configurable later?
    mpCurrBreak->power = 150.0f;
    // Dummy record will instantly be broken
    mpBestBreak->frame = INT_MAX;

    mIsReplay = false;
}

/**
 * @brief Scene reset (before) callback
 */
void Simulation::BeforeReset(RPSysScene* scene) {
#pragma unused(scene)

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

    // 50% chance to aim up, 50% chance to aim sideways
    if (random.Chance(0.5f)) {
        // Randomize aiming UP frames -> [0f, 35f]
        mTimerUp = mpCurrBreak->up = random.NextU32(35);
    } else {
        // Randomize aiming SIDEWAYS frames -> [0f, 12f]
        // 50% chance to aim left vs. aim right
        if (random.Chance(0.5f)) {
            mTimerLeft = mpCurrBreak->left = random.NextU32(12);
        } else {
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
        wiiCtrl.Raw().pos.x =
            mIsReplay ? mpBestBreak->pos.x : mpCurrBreak->pos.x;

        wiiCtrl.Raw().pos.y =
            mIsReplay ? mpBestBreak->pos.y : mpCurrBreak->pos.y;
    }
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

    // Write every break to file
    mpCurrBreak->Save("last.brk");

    mIsReplay = mpCurrBreak->IsBetterThan(*mpBestBreak);
    if (mIsReplay) {
        // Record best shot
        std::memcpy(mpBestBreak, mpCurrBreak, sizeof(BreakInfo));
        mpBestBreak->Log();

        // Write best break to file
        mpBestBreak->Save("best.brk");
    }
}

} // namespace bah