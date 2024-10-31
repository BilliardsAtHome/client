#ifndef BAH_CLIENT_HOOKS_BIL_MAIN_H
#define BAH_CLIENT_HOOKS_BIL_MAIN_H

// Implementation visible to this class
#define private protected
#include <Pack/RPParty/RPBilScene/RPBilMain.h>
#undef private

#include <types.h>

namespace BAH {

/**
 * @brief Billiards game manager ("main" class)
 */
class BilMain : private RPBilMain {
public:
    /**
     * @brief Shot end callback
     */
    void OnEndShot();
};

} // namespace BAH

#endif
