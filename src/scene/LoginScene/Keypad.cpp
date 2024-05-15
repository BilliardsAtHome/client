#include "scene/LoginScene/Keypad.h"

#include <libkiwi.h>

namespace BAH {
namespace {

const f32 BUFFER_Y = 0.5f;
const f32 KEY_SIZE = 0.25f;
const f32 KEYPAD_X = -((KEY_SIZE * 3.0f) / 3.0f);
const f32 KEYPAD_Y = 0.2f;

} // namespace

/**
 * @brief Constructor
 */
Keypad::Keypad() {
    for (int i = 0; i <= EKey_9; i++) {
        mKeys[i].code = '1' + i;
        mKeys[i].callback = NumericKeyCallback;
        mKeys[i].arg = this;
    }

    mKeys[EKey_0].code = '0';
    mKeys[EKey_0].callback = NumericKeyCallback;
    mKeys[EKey_0].arg = this;

    mKeys[EKey_Back].code = "Back";
    mKeys[EKey_Back].callback = BackKeyCallback;
    mKeys[EKey_Back].arg = this;

    mKeys[EKey_Ok].code = "OK";
    mKeys[EKey_Ok].callback = OkKeyCallback;
    mKeys[EKey_Ok].arg = this;

    Reset();
}

/**
 * @brief Update reset
 */
void Keypad::Reset() {
    mBuffer = "";
    mSelectedKey = 0;
}

/**
 * @brief Update logic
 */
void Keypad::Calculate() {
    kiwi::WiiCtrl& ctrl =
        kiwi::CtrlMgr::GetInstance().GetWiiCtrl(kiwi::EPlayer_1);

    if (!ctrl.Connected()) {
        return;
    }

    // Assume we probably will move the cursor
    mKeys[mSelectedKey].hover = false;

    // Move right/left (column change)
    if (ctrl.Trig() & kiwi::WiiCtrl::EButton_Left) {
        mSelectedKey = kiwi::Max<s32>(0, mSelectedKey - 1);

    } else if (ctrl.Trig() & kiwi::WiiCtrl::EButton_Right) {
        mSelectedKey = kiwi::Min<s32>(mKeys.Length() - 1, mSelectedKey + 1);
    }

    // Move up/down (row change)
    if (ctrl.Trig() & kiwi::WiiCtrl::EButton_Up) {
        mSelectedKey = kiwi::Max<s32>(0, mSelectedKey - 3);
    } else if (ctrl.Trig() & kiwi::WiiCtrl::EButton_Down) {
        mSelectedKey = kiwi::Min<s32>(mKeys.Length() - 1, mSelectedKey + 3);
    }

    // Now the cursor reflects the right one
    mKeys[mSelectedKey].hover = true;

    // A button to press key
    if (ctrl.Trig() & kiwi::WiiCtrl::EButton_A) {
        Key& k = mKeys[mSelectedKey];

        if (k.callback != NULL) {
            k.callback(k.code, k.arg);
        }
    }
}

/**
 * @brief User-level draw
 */
void Keypad::UserDraw() const {
    f32 x = KEYPAD_X;
    f32 y = KEYPAD_Y;

    // Draw keys
    for (int i = 0; i < mKeys.Length(); i++) {
        const Key& k = mKeys[i];

        // Emphasize selected key
        f32 scale = k.hover ? 1.25f : 1.0f;
        kiwi::Color color = k.hover ? kiwi::Color::RED : kiwi::Color::WHITE;

        kiwi::DebugPrint::PrintfOutline(x, y, 1.5f, true, color,
                                        kiwi::Color::GREY, k.code);

        // Next column
        x += KEY_SIZE;

        // Next row
        if (i > 0 && ((i + 1) % 3) == 0) {
            y -= KEY_SIZE;
            x = KEYPAD_X;
        }
    }

    // Draw buffer
    kiwi::DebugPrint::PrintfOutline(0.0f, BUFFER_Y, 1.5f, true,
                                    kiwi::Color::WHITE, kiwi::Color::GREY,
                                    mBuffer);
}

/**
 * @brief Numeric keypress callback
 *
 * @param code Key code
 * @param arg Parent keypad
 */
void Keypad::NumericKeyCallback(const kiwi::String& code, void* arg) {
    Keypad* self = static_cast<Keypad*>(arg);
    ASSERT(self != NULL);

    // Append key code
    if (self->mBuffer.Length() + code.Length() <= BUFFER_LEN) {
        self->mBuffer += code;
    }
}

/**
 * @brief Backspace keypress callback
 *
 * @param code Key code
 * @param arg Parent keypad
 */
void Keypad::BackKeyCallback(const kiwi::String& code, void* arg) {
#pragma unused(code)

    Keypad* self = static_cast<Keypad*>(arg);
    ASSERT(self != NULL);

    // Trim off one character
    if (!self->mBuffer.Empty()) {
        self->mBuffer = self->mBuffer.SubStr(0, self->mBuffer.Length() - 1);
    }
}

/**
 * @brief OK keypress callback
 *
 * @param code Key code
 * @param arg Parent keypad
 */
void Keypad::OkKeyCallback(const kiwi::String& code, void* arg) {
#pragma unused(code)

    Keypad* self = static_cast<Keypad*>(arg);
    ASSERT(self != NULL);

    if (self->mpCallback != NULL) {
        self->mpCallback(self->mBuffer, self->mpCallbackArg);
    }
}

} // namespace BAH