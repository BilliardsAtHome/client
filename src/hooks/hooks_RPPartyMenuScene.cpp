#include <types.h>

namespace bah {
namespace {

/**
 * @brief Automatically enter billiards
 */
KM_WRITE_32(0x801e43d0, 0x2C030000);
KM_WRITE_32(0x801e4460, 0x38800014);

} // namespace
} // namespace bah