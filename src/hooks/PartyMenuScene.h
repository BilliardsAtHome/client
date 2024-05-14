#ifndef BAH_CLIENT_HOOKS_PARTY_MENU_SCENE_H
#define BAH_CLIENT_HOOKS_PARTY_MENU_SCENE_H
#include <Pack/RPParty.h>
#include <types.h>

namespace BAH {

/**
 * @brief Party Pack menu scene
 */
class PartyMenuScene : private RPPartyMenuScene {
public:
    void CalculateEx();
};

} // namespace BAH

#endif