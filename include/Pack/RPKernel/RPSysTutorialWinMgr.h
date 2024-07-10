#ifndef RP_KERNEL_TUTORIAL_WINDOW_MGR_H
#define RP_KERNEL_TUTORIAL_WINDOW_MGR_H
#include "types.h"

class RPSysTutorialWinMgr {
public:
    static RPSysTutorialWinMgr* getInstance() {
        return spInstance;
    }

    void LoadResource();

private:
    static RPSysTutorialWinMgr* spInstance;
};

#endif
