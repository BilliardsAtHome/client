#ifndef LIBKIWI_DEBUG_TEXT_WRITER_H
#define LIBKIWI_DEBUG_TEXT_WRITER_H
#include <libkiwi/core/kiwiColor.h>
#include <libkiwi/k_types.h>
#include <libkiwi/prim/kiwiString.h>
#include <libkiwi/util/kiwiStaticSingleton.h>

#include <nw4r/ut.h>

namespace kiwi {
//! @addtogroup libkiwi_debug
//! @{

// Forward declarations
struct Style;

/**
 * @brief Screen text writer
 */
class TextWriter : public StaticSingleton<TextWriter>,
                   public nw4r::ut::TextWriterBase<wchar_t> {

    friend class StaticSingleton<TextWriter>;

public:
    /**
     * @brief Sets the text font
     * @details The provided font must be one available in the font manager
     * (@see RPSysFontManager).
     *
     * @param rName Font name
     * @return Success
     */
    bool SetFont(const kiwi::String& rName);
    /**
     * @brief Sets the text font
     *
     * @param rFont Font resource
     * @return Success
     */
    bool SetFont(const nw4r::ut::Font& rFont);

    /**
     * @brief Prints formatted text to the screen (internal implementation)
     * @details XY coordinates are normalized so they appear the same across
     * aspect ratios.
     *
     * @param x X position [0.0 - 1.0]
     * @param y Y position [0.0 - 1.0]
     * @param rStyle Text style
     * @param rStr Text string
     */
    template <typename T>
    void Print(f32 x, f32 y, const Style& rStyle,
               const kiwi::StringImpl<T>& rStr);

private:
    //!< Base text scale applied to styles
    static const f32 BASE_FONT_SCALE;

private:
    /**
     * @brief Constructor
     */
    TextWriter();
    /**
     * @brief Destructor
     */
    virtual ~TextWriter() {
        ClearFont();
    }

    /**
     * @brief Release font memory and resources
     */
    void ClearFont();

    /**
     * @brief Sets up GX rendering state
     */
    void SetupGX();
};

/**
 * @name Shorthand Functions
 * @brief Use these to quickly call text writer functions
 */
/**@{*/
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
          ...);
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
void Text(f32 x, f32 y, const Style& rStyle, const T* pFmt, ...);

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
          std::va_list args);
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
void Text(f32 x, f32 y, const Style& rStyle, const T* pFmt, std::va_list args);
/**@}*/

/**
 * @name Text styling
 * @brief Use these to configure the text's appearance
 */
/**@{*/
/**
 * @brief Text draw flag
 */
enum ETextFlag {
    //! (Default) Align text to the left of the screen
    ETextFlag_TextLeft = 0,

    //! Align text to the middle of the screen
    ETextFlag_TextCenter = nw4r::ut::DRAWFLAG_ALIGN_H_CENTER,

    //! Align text to the right of the screen
    ETextFlag_TextRight = nw4r::ut::DRAWFLAG_ALIGN_H_RIGHT,
};

/**
 * @brief Text style
 */
struct Style {
    f32 scale;   //!< Text scale
    Color color; //!< Text color
    u32 flags;   //!< Text draw flags

    /**
     * @name Style constructors
     * @brief Use these to pass any configuration
     */
    /**@{*/
    // clang-format off
    
    /* No properties    */
             Style() : scale(1.0f), color(Color::WHITE), flags(ETextFlag_TextLeft) {}
             
    /* Single property  */
    explicit Style(f32   _scale) : scale(_scale), color(Color::WHITE), flags(ETextFlag_TextLeft) { K_ASSERT(scale > 0.0f); }
    explicit Style(Color _color) : scale(1.0f),   color(_color),       flags(ETextFlag_TextLeft) {}
    explicit Style(u32   _flags) : scale(1.0f),   color(Color::WHITE), flags(_flags)             {}
             
    /* Two properties   */
             Style(f32   _scale, Color _color) : scale(_scale), color(_color),       flags(ETextFlag_TextLeft) { K_ASSERT(scale > 0.0f); }
             Style(f32   _scale, u32   _flags) : scale(_scale), color(Color::WHITE), flags(_flags)             { K_ASSERT(scale > 0.0f); }
             Style(Color _color, u32   _flags) : scale(1.0f),   color(_color),       flags(_flags)             {}
    
    /* Three properties */
             Style(f32 _scale, Color _color, u32 _flags) : scale(_scale), color(_color), flags(_flags) { K_ASSERT(scale > 0.0f); }

    // clang-format on
    /**@}*/
};
/**@}*/

//! @}
} // namespace kiwi

#endif
