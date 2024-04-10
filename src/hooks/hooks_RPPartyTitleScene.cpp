#include <types.h>

namespace BAH {
namespace {

/**
 * @brief Automatically skip A/B press
 */
asm void title_scene_skip_ab(){
    // clang-format off
    nofralloc

    // Check for wait state
    lwz r0, 0x44(r31)
    cmpwi r0, 19
    bne _epilogue

    // Patch to exit state
    li r0, 0
    stw r0, 0x44(r31)

_epilogue:
    // Original function epilogue
    opword 0x80010024
    opword 0x83e1001c
    opword 0x7c0803a6
    opword 0x38210020
    blr

    // clang-format on
} KM_BRANCH(0x801e4074, title_scene_skip_ab);

} // namespace
} // namespace BAH