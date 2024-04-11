#ifndef LIBKIWI_GUI_IMGUI_USER_CONFIG_H
#define LIBKIWI_GUI_IMGUI_USER_CONFIG_H
#include <libkiwi/kernel/kiwiAssert.h>
#include <libkiwi/gui/kiwiImGuiImplRvl.h>
#include <types.h>

// Disable incompatible functionality
#define IMGUI_DISABLE_WIN32_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS
#define IMGUI_DISABLE_SSE

// Redirect file operations to NAND
#define IMGUI_DISABLE_FILE_FUNCTIONS

// Redirect ImGui assertions
#define IM_ASSERT(expr) K_ASSERT(expr)
// Redirect debugger break
#define IM_DEBUG_BREAK() IM_ASSERT(0)

// STL qsort not linked in our games
#define ImQsort kiwi::ImQsort

#endif