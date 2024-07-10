#ifndef RP_KERNEL_EFFECT_MGR_H
#define RP_KERNEL_EFFECT_MGR_H
#include "types.h"

class RPSysEffectMgr {
public:
    static RPSysEffectMgr* getInstance() {
        return spInstance;
    }

    void LoadResource();

private:
    static RPSysEffectMgr* spInstance;
};

#endif
