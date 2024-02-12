#include <types.h>

namespace bah {
namespace {

/**
 * @brief Automatically skip strap screen
 */
KM_WRITE_32(0x801cb5c8, 0x60000000);

} // namespace
} // namespace bah