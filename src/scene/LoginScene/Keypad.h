#ifndef BAH_CLIENT_SCENE_LOGIN_SCENE_KEYPAD_H
#define BAH_CLIENT_SCENE_LOGIN_SCENE_KEYPAD_H
#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Unique ID keypad
 */
class Keypad {
private:
    /**
     * @brief Number key
     */
    struct Key {
        typedef void (*Callback)(const kiwi::String& code, void* arg);

        kiwi::String code; // Key code
        bool hover;        // Hovered status

        Callback callback; // Press callback
        void* arg;         // Callback argument

        /**
         * @brief Constructor
         */
        Key() : code(' '), hover(false), callback(NULL), arg(NULL) {}
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

        EKey_Back,
        EKey_0,
        EKey_Ok,

        EKey_Max
    };

public:
    typedef void (*OkCallback)(const kiwi::String& result, void* arg);

public:
    Keypad();

    void SetOkCallback(OkCallback callback, void* arg = NULL) {
        mpCallback = callback;
        mpCallbackArg = arg;
    }

    void Reset();
    void Calculate();
    void UserDraw() const;

private:
    static void NumericKeyCallback(const kiwi::String& code, void* arg);
    static void BackKeyCallback(const kiwi::String& code, void* arg);
    static void OkKeyCallback(const kiwi::String& code, void* arg);

private:
    static const u32 BUFFER_LEN = 8;

    kiwi::TArray<Key, EKey_Max> mKeys; // Key set
    u32 mSelectedKey;                  // Selected key index
    kiwi::String mBuffer;              // Key buffer

    OkCallback mpCallback; // OK keypress callback
    void* mpCallbackArg;   // OK keypress callback argument
};

} // namespace BAH

#endif