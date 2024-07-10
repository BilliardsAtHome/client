#ifndef RP_SYSTEM_PAUSE_MGR_H
#define RP_SYSTEM_PAUSE_MGR_H
#include "types.h"

class RPSysPauseMgr {
public:
    static RPSysPauseMgr* getInstance() {
        return spInstance;
    }

    void LoadResource();

private:
    static RPSysPauseMgr* spInstance;
};

#endif
