#ifndef RP_PARTY_GAME_MGR_H
#define RP_PARTY_GAME_MGR_H
#include "types.h"

class RPPartyGameMgr {
public:
    static RPPartyGameMgr* getInstance() {
        return spInstance;
    }

    static void CreateInstance();
    static void DestroyInstance();

    void Reset();

private:
    static RPPartyGameMgr* spInstance;
};

#endif
