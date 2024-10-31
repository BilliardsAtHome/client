#ifndef BAH_CLIENT_HOOKS_BIL_CUE_H
#define BAH_CLIENT_HOOKS_BIL_CUE_H

// Implementation visible to this class
#define private protected
#include <Pack/RPParty/RPBilScene/RPBilCue.h>
#undef private

#include <types.h>

namespace BAH {

/**
 * @brief Billiards cue
 */
class BilCue : private RPBilCue {
public:
    /**
     * @brief Logic step
     */
    void CalculateEx();

    /**
     * @brief HOLD state logic
     */
    void State_HOLD_calc_Ex();
    /**
     * @brief WAIT state logic
     */
    void State_WAIT_calc_Ex();
};

} // namespace BAH

#endif
