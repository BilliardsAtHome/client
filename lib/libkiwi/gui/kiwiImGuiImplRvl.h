#ifndef LIBKIWI_GUI_IMGUI_IMPL_RVL_H
#define LIBKIWI_GUI_IMGUI_IMPL_RVL_H
#include <types.h>

namespace kiwi {

typedef int (*ImQCompareFunc)(const void* a, const void* b);

void ImQsort(void* base, u32 count, u32 size, ImQCompareFunc func);

} // namespace kiwi

#endif