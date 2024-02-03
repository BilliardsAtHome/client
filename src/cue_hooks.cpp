#include "Simulation.hpp"

namespace bah {
namespace {

/**
 * @brief Force WAIT -> HOLD transition
 */
bool Cue_CanHold() {
    return Simulation::GetInstance().IsDoneAiming();
}
KM_CALL(0x802bfbe0, Cue_CanHold);
KM_WRITE_32(0x802bfbe4, 0x2C030000);

/**
 * @brief Force HOLD -> PULL transition
 */
KM_WRITE_32(0x802bf88c, 0x4800002C);
KM_WRITE_32(0x802bf8cc, 0x60000000);

/**
 * @brief Force PULL -> HIT transition
 */
KM_WRITE_32(0x802be88c, 0x4800002C);
KM_WRITE_32(0x802be9a0, 0x60000000);
KM_WRITE_32(0x802be9ac, 0x60000000);
KM_WRITE_32(0x802be9b8, 0x60000000);
KM_WRITE_32(0x802bead0, 0x60000000);

/**
 * @brief Fixes for controller disconnect
 */
KM_WRITE_32(0x802bfbc4, 0x60000000);
KM_WRITE_32(0x802bfbf4, 0x60000000);
KM_WRITE_32(0x802bfc3c, 0x60000000);

/**
 * @brief Redirect cue power calculation to the simulated value
 */
f32 Cue_GetPower() {
    return Simulation::GetInstance().GetCuePower();
}
KM_CALL(0x802beff8, Cue_GetPower);
KM_CALL(0x802bf070, Cue_GetPower);
KM_CALL(0x802bf080, Cue_GetPower);

} // namespace
} // namespace bah