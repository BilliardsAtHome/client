#ifndef BIL_MAIN_H
#define BIL_MAIN_H
#include <Pack/RPParty.h>
#include <Pack/RPUtility.h>
#include <libkiwi.h>
#include <types.h>

class BilMain : kiwi::Override<RPBilMain> {
public:
    RP_UTL_FSM_STATE_DECL(AFTERSHOT);
    RP_UTL_FSM_STATE_DECL(FOUL);

    void OnEndShot();

private:
    RPUtlBaseFsm<BilMain>* mpStateMachine;
};

#endif