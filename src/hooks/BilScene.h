#ifndef BAH_CLIENT_HOOKS_BIL_SCENE_H
#define BAH_CLIENT_HOOKS_BIL_SCENE_H
#include <Pack/RPParty.h>
#include <types.h>

namespace BAH {

/**
 * @brief Billiards scene
 */
class BilScene : private RPBilScene {
public:
    void CalculateEx();
};

} // namespace BAH

#endif