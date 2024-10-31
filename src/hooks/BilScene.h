#ifndef BAH_CLIENT_HOOKS_BIL_SCENE_H
#define BAH_CLIENT_HOOKS_BIL_SCENE_H

// Implementation visible to this class
#define private protected
#include <Pack/RPParty/RPBilScene/RPBilScene.h>
#undef private

#include <types.h>

namespace BAH {

/**
 * @brief Billiards scene
 */
class BilScene : private RPBilScene {
public:
    /**
     * @brief Logic step
     */
    void CalculateEx();
};

} // namespace BAH

#endif
