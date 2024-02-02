#ifndef BIL_CUE_H
#define BIL_CUE_H
#include <Pack/RPParty.h>
#include <Pack/RPUtility.h>
#include <libkiwi.h>
#include <types.h>

class BilCue : kiwi::Override<RPBilCue> {
public:
public:
    RP_UTL_FSM_STATE_DECL(HOLD);

private:
    char _00[0x68];
    RPUtlBaseFsm<BilCue>* mpStateMachine;
};

#endif