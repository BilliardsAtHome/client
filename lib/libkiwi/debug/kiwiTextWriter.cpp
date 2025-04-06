#include <Pack/RPKernel.h>

#include <libkiwi.h>

#include <nw4r/math.h>

#include <revolution/GX.h>

namespace kiwi {

/**
 * @brief Constructor
 */
TextWriter::TextWriter() : mIsRendering(false), mOldDrawFlags(0) {
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
bool TextWriter::SetFont(const String& rName) {
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
    nw4r::ut::WideTextWriter::SetFont(rFont);
    return true;
}

/**
 * @brief Sets up GX rendering state for printing
 */
void TextWriter::Begin() {
    SetupGX();
    mIsRendering = true;
    mOldDrawFlags = GetDrawFlag();
}

/**
 * @brief Tears down GX rendering state
 */
void TextWriter::End() {
    mIsRendering = false;
    SetDrawFlag(mOldDrawFlags);
}

/**
 * @brief Prints formatted text to the screen
 * @details XY coordinates are normalized so they appear the same across
 * aspect ratios.
 *
 * @param x X position [0.0 - 1.0]
 * @param y Y position [0.0 - 1.0]
 * @param rStr Text string (UTF-16)
 */
template <typename T>
void TextWriter::Print(f32 x, f32 y, const StringImpl<T>& rStr) {
    K_ASSERT_EX(mIsRendering, "Please call TextWriter::Begin before printing");

    K_ASSERT_EX(0.0f <= x && x <= 1.0f,
                "X position is out of range [0, 1] (%.2f)", x);
    K_ASSERT_EX(0.0f <= y && y <= 1.0f,
                "Y position is out of range [0, 1] (%.2f)", y);

    // Don't clobber unrelated renderer passes
    if (!RPGrpRenderer::IsLytDraw()) {
        return;
    }

    // Convert to screen pixels
    x *= RPGrpScreen::GetSizeXMax();
    y *= RPGrpScreen::GetSizeYMax();

    // Correct for centered canvas mode
    if (RPGrpRenderer::GetActiveScreen()->GetCanvasMode() ==
        RPGrpScreen::CANVASMODE_CC) {

        // Convert to top-left origin
        x -= RPGrpScreen::GetSizeXMax() / 2.0f;
        y -= RPGrpScreen::GetSizeYMax() / 2.0f;
    }

    SetCursor(x, y);

    // ASCII text must be converted to UTF-16
    nw4r::ut::WideTextWriter::Print(rStr.ToWideChar(), rStr.Length());
}

/**
 * @brief Release font memory and resources
 */
void TextWriter::ClearFont() {
    const nw4r::ut::Font* pFont = nw4r::ut::WideTextWriter::GetFont();
    if (pFont == nullptr) {
        return;
    }

    // ROM font is owned by the font manager
    if (pFont == RP_GET_INSTANCE(RPSysFontManager)->GetRomFont()) {
        return;
    }

    delete pFont;
}

/**
 * @brief Sets up GX rendering state
 */
void TextWriter::SetupGX() {
    nw4r::ut::WideTextWriter::SetupGX();

    GXSetZMode(FALSE, GX_ALWAYS, FALSE);

    nw4r::math::MTX34 posMtx;
    nw4r::math::MTX34Identity(&posMtx);

    // Correct for centered canvas mode
    if (RPGrpRenderer::GetActiveScreen()->GetCanvasMode() ==
        RPGrpScreen::CANVASMODE_CC) {

        posMtx._01 = -posMtx._01;
        posMtx._11 = -posMtx._11;
        posMtx._21 = -posMtx._21;
    }

    GXLoadPosMtxImm(posMtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

// Instantiate function templates
template void TextWriter::Print<char>(f32 x, f32 y,
                                      const StringImpl<char>& rStr);
template void TextWriter::Print<wchar_t>(f32 x, f32 y,
                                         const StringImpl<wchar_t>& rStr);

} // namespace kiwi
