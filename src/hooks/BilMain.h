#ifndef BAH_CLIENT_HOOKS_BIL_MAIN_H
#define BAH_CLIENT_HOOKS_BIL_MAIN_H
#include <Pack/RPParty.h>
#include <types.h>

namespace BAH {

/**
 * @brief Billiards game manager ("main" class)
 */
class BilMain : private RPBilMain {
public:
    void OnEndShot();
};

} // namespace BAH

#endif