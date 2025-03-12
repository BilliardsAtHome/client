#include <Pack/RPKernel.h>

#include <libkiwi.h>

#include <egg/core.h>

#include <nw4r/math.h>

#include <cstring>

namespace kiwi {

/**
 * @brief Base text scale applied to styles
 */
const f32 TextWriter::BASE_FONT_SCALE = 0.75f;

/**
 * @brief Constructor
 */
TextWriter::TextWriter() {
    bool success = SetFont("pac_nRodDb_32_I4.brfnt");
    K_ASSERT_EX(success, "Could not prepare resource font");
}

/**
 * @brief Sets the text font
 * @details The provided font must be one available in the font manager
 * (@see RPSysFontManager).
 *
 * @param rName Font name
 * @return Success
 */
bool TextWriter::SetFont(const kiwi::String& rName) {
    void* pFontData = RP_GET_INSTANCE(RPSysFontManager)->GetResFontData(rName);
    K_ASSERT(pFontData != nullptr);

    nw4r::ut::ResFont* pFont = new nw4r::ut::ResFont();
    K_ASSERT(pFont != nullptr);

    bool success = pFont->SetResource(pFontData);
    K_ASSERT(success);

    // Keep existing font data on failure
    if (success) {
        SetFont(*pFont);
    } else {
        delete pFont;
    }

    return success;
}

/**
 * @brief Sets the text font
 *
 * @param rFont Font resource
 * @return Success
 */
bool TextWriter::SetFont(const nw4r::ut::Font& rFont) {
    K_ASSERT_EX(rFont.GetEncoding() == nw4r::ut::FONT_ENCODING_UTF16,
                "Only UTF-16 fonts are supported");

    if (&rFont == GetFont()) {
        return true;
    }

    if (rFont.GetEncoding() != nw4r::ut::FONT_ENCODING_UTF16) {
        return false;
    }

    ClearFont();
    nw4r::ut::TextWriterBase<wchar_t>::SetFont(rFont);
    return true;
}

/**
 * @brief Prints formatted text to the screen (internal implementation)
 * @details XY coordinates are normalized so they appear the same across
 * aspect ratios.
 *
 * @param x X position [0.0 - 1.0]
 * @param y Y position [0.0 - 1.0]
 * @param rStyle Text style
 * @param rStr Text string (UTF-16)
 */
template <typename T>
void TextWriter::Print(f32 x, f32 y, const Style& rStyle,
                       const kiwi::StringImpl<T>& rStr) {

    K_ASSERT_EX(0.0f <= x && x <= 1.0f,
                "X position is out of range [0, 1] (%.2f)", x);
    K_ASSERT_EX(0.0f <= y && y <= 1.0f,
                "Y position is out of range [0, 1] (%.2f)", y);

    K_ASSERT(rStyle.scale > 0.0f);

    // Don't clobber unrelated renderer passes
    if (!RPGrpRenderer::IsLytDraw()) {
        return;
    }

    // Convert to screen pixels
    x *= EGG::Screen::GetSizeXMax();
    y *= EGG::Screen::GetSizeYMax();

    // Correct for centered canvas mode
    if (RPGrpRenderer::GetActiveScreen()->GetCanvasMode() ==
        EGG::Frustum::CANVASMODE_CC) {

        // Convert to top-left origin
        x -= EGG::Screen::GetSizeXMax() / 2.0f;
        y -= EGG::Screen::GetSizeYMax() / 2.0f;
    }

    SetCursor(x, y);
    SetScale(rStyle.scale * BASE_FONT_SCALE, rStyle.scale * BASE_FONT_SCALE);
    SetTextColor(rStyle.color);

    u32 drawFlag = GetDrawFlag();
    SetDrawFlag(rStyle.flags);

    SetupGX();

    // ASCII text must be converted to UTF-16
    nw4r::ut::TextWriterBase<wchar_t>::Print(rStr.ToWideChar(), rStr.Length());

    SetDrawFlag(drawFlag);
}

/**
 * @brief Release font memory and resources
 */
void TextWriter::ClearFont() {
    const nw4r::ut::Font* pFont = nw4r::ut::TextWriterBase<wchar_t>::GetFont();
    if (pFont == nullptr) {
        return;
    }

    // ROM font is owned by the font manager
    if (pFont != RP_GET_INSTANCE(RPSysFontManager)->GetRomFont()) {
        delete pFont;
    }
}

/**
 * @brief Sets up GX rendering state
 */
void TextWriter::SetupGX() {
    nw4r::ut::TextWriterBase<wchar_t>::SetupGX();

    GXSetZMode(FALSE, GX_ALWAYS, FALSE);

    nw4r::math::MTX34 posMtx;
    nw4r::math::MTX34Identity(&posMtx);

    // Correct for centered canvas mode
    if (RPGrpRenderer::GetActiveScreen()->GetCanvasMode() ==
        EGG::Frustum::CANVASMODE_CC) {

        posMtx._01 = -posMtx._01;
        posMtx._11 = -posMtx._11;
        posMtx._21 = -posMtx._21;
    }

    GXLoadPosMtxImm(posMtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

/**
 * @brief Prints formatted text to the screen
 * @details XY coordinates are normalized so they appear the same across aspect
 * ratios.
 *
 * @param x X position [0.0 - 1.0]
 * @param y Y position [0.0 - 1.0]
 * @param rStyle Text style
 * @param rFmt Format string
 * @param ... Format arguments
 */
template <typename T>
void Text(f32 x, f32 y, const Style& rStyle, const kiwi::StringImpl<T>& rFmt,
          ...) {

    std::va_list list;
    va_start(list, rFmt);
    StringImpl<T> str = VFormat(rFmt, list);
    va_end(list);

    TextWriter::GetInstance().Print(x, y, rStyle, str);
}

/**
 * @brief Prints formatted text to the screen
 * @details XY coordinates are normalized so they appear the same across aspect
 * ratios.
 *
 * @param x X position [0.0 - 1.0]
 * @param y Y position [0.0 - 1.0]
 * @param rStyle Text style
 * @param pFmt Format string
 * @param ... Format arguments
 */
template <typename T>
void Text(f32 x, f32 y, const Style& rStyle, const T* pFmt, ...) {
    std::va_list list;
    va_start(list, pFmt);
    StringImpl<T> str = VFormat(pFmt, list);
    va_end(list);

    TextWriter::GetInstance().Print(x, y, rStyle, str);
}

/**
 * @brief Prints formatted text to the screen
 * @details XY coordinates are normalized so they appear the same across aspect
 * ratios.
 *
 * @param x X position [0.0 - 1.0]
 * @param y Y position [0.0 - 1.0]
 * @param rStyle Text style
 * @param rFmt Format string
 * @param args Format arguments
 */

template <typename T>
void Text(f32 x, f32 y, const Style& rStyle, const kiwi::StringImpl<T>& rFmt,
          std::va_list args) {

    StringImpl<T> str = VFormat(rFmt, args);
    TextWriter::GetInstance().Print(x, y, rStyle, str);
}

/**
 * @brief Prints formatted text to the screen
 * @details XY coordinates are normalized so they appear the same across aspect
 * ratios.
 *
 * @param x X position [0.0 - 1.0]
 * @param y Y position [0.0 - 1.0]
 * @param rStyle Text style
 * @param pFmt Format string
 * @param ... Format arguments
 */

template <typename T>
void Text(f32 x, f32 y, const Style& rStyle, const T* pFmt, std::va_list args) {
    StringImpl<T> str = VFormat(pFmt, args);
    TextWriter::GetInstance().Print(x, y, rStyle, str);
}

// clang-format off

// Instantiate functions
template void TextWriter::Print<char>(f32 x, f32 y, const Style& rStyle, const kiwi::StringImpl<char>& rStr);
template void TextWriter::Print<wchar_t>(f32 x, f32 y, const Style& rStyle, const kiwi::StringImpl<wchar_t>& rStr);

template void Text<char>(f32 x, f32 y, const Style& rStyle, const kiwi::StringImpl<char>& rFmt, ...);
template void Text<char>(f32 x, f32 y, const Style& rStyle, const char* pFmt, ...);
template void Text<char>(f32 x, f32 y, const Style& rStyle, const kiwi::StringImpl<char>& rFmt, std::va_list args);
template void Text<char>(f32 x, f32 y, const Style& rStyle, const char* pFmt, std::va_list args);

template void Text<wchar_t>(f32 x, f32 y, const Style& rStyle, const kiwi::StringImpl<wchar_t>& rFmt, ...);
template void Text<wchar_t>(f32 x, f32 y, const Style& rStyle, const wchar_t* pFmt, ...);
template void Text<wchar_t>(f32 x, f32 y, const Style& rStyle, const kiwi::StringImpl<wchar_t>& rFmt, std::va_list args);
template void Text<wchar_t>(f32 x, f32 y, const Style& rStyle, const wchar_t* pFmt, std::va_list args);

// clang-format on

} // namespace kiwi
