#ifndef BAH_CLIENT_HOOKS_BIL_CUE_H
#define BAH_CLIENT_HOOKS_BIL_CUE_H

// Implementation visible to this class
#define private protected
#include <Pack/RPParty/RPBilScene/RPBilCue.h>
#undef private

#include <Pack/RPParty.h>
#include <types.h>

namespace BAH {

/**
 * @brief Billiards cue
 */
class BilCue : private RPBilCue {
public:
    void CalculateEx();

    void State_HOLD_calc_Ex();
    void State_WAIT_calc_Ex();
};

} // namespace BAH

#endif