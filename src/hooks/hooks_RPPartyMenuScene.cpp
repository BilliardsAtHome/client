#include <types.h>

namespace BAH {
namespace {

/**
 * @brief Automatically enter the login scene
 */
KM_WRITE_32(0x801e43d0, 0x2C030000);
KM_WRITE_32(0x801e4460, 0x3880001C);

} // namespace
} // namespace BAH