#ifndef RP_PARTY_BIL_CUE_H
#define RP_PARTY_BIL_CUE_H
#include <Pack/RPParty/RPPartyUtlModel.h>
#include <Pack/RPUtility/RPUtlBaseFsm.h>
#include <egg/core.h>
#include <egg/gfx.h>
#include <types.h>

/**
 * @brief Billiards cue
 */
class RPBilCue : public RPPartyUtlModel {
public:
    enum EState {
        EState_Null,
        EState_PlaceFoul,
        EState_Wait,
        EState_Hold,
        EState_Pull,
        EState_FastTurn,
        EState_Hit,
        EState_Dummy
    };

    enum ECursor {
        ECursor_AimHover,
        ECursor_AimLock,
        ECursor_AimMiss,
        ECursor_CamHover,
        ECursor_CamLock,
    };

public:
    bool IsWait() const {
        return mpStateMachine->IsState(EState_Wait);
    }

    void SetAimPosition(const EGG::Vector2f& pos) {
        mAimPosition = pos;
        mValidAim = true;
    }

    void CalcAimPosition();
    void CalcAimForce();

private:
    char _60[0x8];
    RPUtlBaseFsm<RPBilCue>* mpStateMachine; // at 0x68
    char _6C[0xA4 - 0x6C];

    bool mValidAim;             // at 0xA4
    EGG::Vector2f mAimPosition; // at 0xA8
    f32 mAimDistance;           // at 0xB0

    bool mLastValidAim;             // at 0xB4
    EGG::Vector2f mLastAimPosition; // at 0xB8

    char _C0[0x6878 - 0xC0];
    u32 mNumShot; // at 0x6878
    u32 mNumFoul; // at 0x687C

    char _6880[0x68AC - 0x6880];
    ECursor mCursor; // at 0x68AC
};

#endif
