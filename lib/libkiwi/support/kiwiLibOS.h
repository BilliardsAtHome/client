#ifndef LIBKIWI_SUPPORT_LIBOS_H
#define LIBKIWI_SUPPORT_LIBOS_H
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <revolution/OS.h>

namespace kiwi {

/**
 * @brief OS library wrapper/extension
 */
class LibOS {
public:
    static void FillFPUContext(OSContext* ctx);
};

} // namespace kiwi

#endif
