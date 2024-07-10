#ifndef RP_KERNEL_HOME_MENU_MGR_H
#define RP_KERNEL_HOME_MENU_MGR_H
#include "types.h"

class RPSysHomeMenuMgr {
public:
    static RPSysHomeMenuMgr* getInstance() {
        return spInstance;
    }

    void LoadResource();

private:
    static RPSysHomeMenuMgr* spInstance;
};

#endif
