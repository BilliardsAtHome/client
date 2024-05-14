#ifndef BAH_CLIENT_HOOKS_SYS_BOOT_SCENE_H
#define BAH_CLIENT_HOOKS_SYS_BOOT_SCENE_H
#include <Pack/RPSystem.h>
#include <types.h>

namespace BAH {

/**
 * @brief Pack Project boot scene
 */
class SysBootScene : private RPSysBootScene {
public:
    void CalculateWait();
};

} // namespace BAH

#endif