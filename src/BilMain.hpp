#ifndef BAH_CLIENT_BIL_MAIN_H
#define BAH_CLIENT_BIL_MAIN_H
#include <Pack/RPParty.h>
#include <Pack/RPUtility.h>
#include <libkiwi.h>
#include <types.h>

namespace bah {

/**
 * @brief Billiards game manager ("main" class)
 */
class BilMain : kiwi::Override<RPBilMain> {
public:
    RP_UTL_FSM_STATE_DECL(AFTERSHOT);
    RP_UTL_FSM_STATE_DECL(FOUL);

    void OnEndShot();
};

} // namespace bah

#endif