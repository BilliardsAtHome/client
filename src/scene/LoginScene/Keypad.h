#ifndef BAH_CLIENT_SCENE_LOGIN_SCENE_KEYPAD_H
#define BAH_CLIENT_SCENE_LOGIN_SCENE_KEYPAD_H
#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Numerical keypad
 */
class Keypad {
public:
    /**
     * @brief Submit ('OK') callback
     *
     * @param rResult Keypad result sequence
     * @param pArg User argument
     */
    typedef void (*OkCallback)(const kiwi::String& rResult, void* pArg);

public:
    /**
     * @brief Constructor
     */
    Keypad();

    /**
     * @brief Reset state
     */
    void Reset();
    /**
     * @brief Logic step
     */
    void Calculate();
    /**
     * @brief Standard draw pass
     */
    void UserDraw() const;

    /**
     * @brief Sets the submit ('OK') callback
     *
     * @param pCallback Callback function
     * @param pArg Callback user argument
     */
    void SetOkCallback(OkCallback pCallback, void* pArg = nullptr) {
        mpOkCallback = pCallback;
        mpOkCallbackArg = pArg;
    }

private:
    /**
     * @brief Number key
     */
    struct Key {
        /**
         * @brief Keypress callback
         *
         * @param rCode Key code
         * @param pArg User argument
         */
        typedef void (*Callback)(const kiwi::String& rCode, void* pArg);

        kiwi::String code; //!< Key code
        bool hover;        //!< Hovered status

        Callback pCallback; //!< Press callback
        void* pArg;         //!< Callback argument

        /**
         * @brief Constructor
         */
        Key() : hover(false), pCallback(nullptr), pArg(nullptr) {}
    };

    enum EKey {
        EKey_1,
        EKey_2,
        EKey_3,

        EKey_4,
        EKey_5,
        EKey_6,

        EKey_7,
        EKey_8,
        EKey_9,

        EKey_Back, //!< Backspace
        EKey_0,
        EKey_Ok, //!< Submit/OK

        EKey_Max
    };

    //! Maximum keypad result length
    static const u32 CHARS_MAX = 8;

private:
    /**
     * @brief Numeric key callback
     *
     * @param rCode Key code
     * @param pArg User argument (parent keypad)
     */
    static void NumericKeyCallback(const kiwi::String& rCode, void* pArg);

    /**
     * @brief Backspace key callback
     *
     * @param rCode Key code
     * @param pArg User argument (parent keypad)
     */
    static void BackKeyCallback(const kiwi::String& rCode, void* pArg);

    /**
     * @brief 'OK' key callback
     *
     * @param rCode Key code
     * @param pArg User argument (parent keypad)
     */
    static void OkKeyCallback(const kiwi::String& rCode, void* pArg);

private:
    //! Key set
    kiwi::TArray<Key, EKey_Max> mKeys;

    //! Highlighted key index
    u32 mKeyNo;
    //! Keypad result buffer
    kiwi::String mBuffer;

    //! Submit ('OK') callback
    OkCallback mpOkCallback;
    //! Submit ('OK') callback user argument
    void* mpOkCallbackArg;
};

} // namespace BAH

#endif