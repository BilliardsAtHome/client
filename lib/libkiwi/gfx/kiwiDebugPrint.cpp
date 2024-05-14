#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <libkiwi.h>

namespace kiwi {

/**
 * @brief Prints formatted text to the screen
 *
 * @param x X position
 * @param y Y position
 * @param scale Text scale
 * @param center Center-align text
 * @param color Text color
 * @param fmt Format string
 * @param ... Format arguments
 */
void DebugPrint::Printf(f32 x, f32 y, f32 scale, bool center, kiwi::Color color,
                        const char* fmt, ...) {
    if (!RPGrpRenderer::IsDrawLayout()) {
        return;
    }

    std::va_list list;
    va_start(list, fmt);
    kiwi::String msg = kiwi::VFormat(fmt, list);
    va_end(list);

    PrintImpl(x, y, scale, center, color, msg);
}

/**
 * @brief Prints formatted text to the screen with a shadow
 *
 * @param x X position
 * @param y Y position
 * @param scale Text scale
 * @param center Center-align text
 * @param text Text color
 * @param shadow Shadow color
 * @param fmt Format string
 * @param ... Format arguments
 */
void DebugPrint::PrintfShadow(f32 x, f32 y, f32 scale, bool center,
                              kiwi::Color text, kiwi::Color shadow,
                              const char* fmt, ...) {
    if (!RPGrpRenderer::IsDrawLayout()) {
        return;
    }

    std::va_list list;
    va_start(list, fmt);
    kiwi::String msg = kiwi::VFormat(fmt, list);
    va_end(list);

    // Shadow
    PrintImpl(x + 0.0035f, y + 0.0035f, scale, center, shadow, msg);
    // Text
    PrintImpl(x, y, scale, center, text, msg);
}

/**
 * @brief Prints formatted text to the screen with an outline
 *
 * @param x X position
 * @param y Y position
 * @param scale Text scale
 * @param center Center-align text
 * @param text Text color
 * @param outline Outline color
 * @param fmt Format string
 * @param ... Format arguments
 */
void DebugPrint::PrintfOutline(f32 x, f32 y, f32 scale, bool center,
                               kiwi::Color text, kiwi::Color outline,
                               const char* fmt, ...) {
    if (!RPGrpRenderer::IsDrawLayout()) {
        return;
    }

    std::va_list list;
    va_start(list, fmt);
    kiwi::String msg = kiwi::VFormat(fmt, list);
    va_end(list);

    // Outline
    PrintImpl(x - 0.0035f, y, scale, center, outline, msg);
    PrintImpl(x + 0.0035f, y, scale, center, outline, msg);
    PrintImpl(x, y - 0.0035f, scale, center, outline, msg);
    PrintImpl(x, y + 0.0035f, scale, center, outline, msg);
    // Text
    PrintImpl(x, y, scale, center, text, msg);
}

/**
 * @brief Prints text to the screen
 *
 * @param x X position
 * @param y Y position
 * @param scale Text scale
 * @param center Center-align text
 * @param color Text color
 * @param msg Text message
 */
void DebugPrint::PrintImpl(f32 x, f32 y, f32 scale, bool center,
                           kiwi::Color color, const kiwi::String& msg) {
    RPSysTextWriter::GetInstance()->Begin();
    {
        // Text alignment
        u32 flag = RPSysTextWriter::DrawFlag_AlignCenterY;
        if (center) {
            flag |= RPSysTextWriter::DrawFlag_AlignCenterX;
        }

        u32 oldFlag = RPSysTextWriter::GetInstance()->GetDrawFlag();
        {
            RPSysTextWriter::GetInstance()->SetDrawFlag(flag);
            RPSysTextWriter::GetInstance()->SetTextColor(color);
            RPSysTextWriter::GetInstance()->SetScale(scale, scale);
            RPSysTextWriter::GetInstance()->PrintfZeroCenter(x, y, msg);
        }
        RPSysTextWriter::GetInstance()->SetDrawFlag(oldFlag);
    }
    RPSysTextWriter::GetInstance()->End();
}

} // namespace kiwi