#ifndef RP_SYSTEM_PLAYER_MGR_H
#define RP_SYSTEM_PLAYER_MGR_H
#include "types.h"

class RPSysPlayerMgr {
public:
    static RPSysPlayerMgr* getInstance() {
        return spInstance;
    }

    void resetData();
    void setPlayerNum(int num);
    void setControllerNum(int num);

private:
    static RPSysPlayerMgr* spInstance;
};

#endif
