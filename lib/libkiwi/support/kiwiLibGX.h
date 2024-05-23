#ifndef LIBKIWI_SUPPORT_LIBGX_H
#define LIBKIWI_SUPPORT_LIBGX_H
#include <revolution/GX.h>
#include <types.h>

namespace kiwi {

/**
 * GX library wrapper/extension
 */
class LibGX {
public:
    static const GXRenderModeObj* GetDefaultRenderMode();
};

} // namespace kiwi

#endif
