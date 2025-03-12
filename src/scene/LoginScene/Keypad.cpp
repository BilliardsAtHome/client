#include "scene/LoginScene/Keypad.h"

#include <libkiwi.h>

namespace BAH {
namespace {

const f32 BUFFER_Y = 0.4f;
const f32 KEY_SIZE = 0.2f;
const f32 KEYPAD_X = -((KEY_SIZE * 3.0f) / 3.0f);
const f32 KEYPAD_Y = 0.2f;

} // namespace

/**
 * @brief Constructor
 */
Keypad::Keypad() {
    for (int i = 0; i <= EKey_9; i++) {
        mKeys[i].code = '1' + i;
        mKeys[i].pCallback = NumericKeyCallback;
        mKeys[i].pArg = this;
    }

    mKeys[EKey_0].code = '0';
    mKeys[EKey_0].pCallback = NumericKeyCallback;
    mKeys[EKey_0].pArg = this;

    mKeys[EKey_Back].code = "Back";
    mKeys[EKey_Back].pCallback = BackKeyCallback;
    mKeys[EKey_Back].pArg = this;

    mKeys[EKey_Ok].code = "OK";
    mKeys[EKey_Ok].pCallback = OkKeyCallback;
    mKeys[EKey_Ok].pArg = this;

    Reset();
}

/**
 * @brief Reset state
 */
void Keypad::Reset() {
    mKeyNo = 0;
    mBuffer = "";
}

/**
 * @brief Logic step
 */
void Keypad::Calculate() {
    const kiwi::WiiCtrl& rCtrl =
        kiwi::CtrlMgr::GetInstance().GetWiiCtrl(kiwi::EPlayer_1);

    if (!rCtrl.IsConnected()) {
        return;
    }

    // Assume we probably will move the cursor
    mKeys[mKeyNo].hover = false;

    // Move right/left (column change)
    if (rCtrl.IsTrig(kiwi::EButton_Left)) {
        mKeyNo = kiwi::Max<s32>(0, mKeyNo - 1);

    } else if (rCtrl.IsTrig(kiwi::EButton_Right)) {
        mKeyNo = kiwi::Min<s32>(mKeys.Length() - 1, mKeyNo + 1);
    }

    // Move up/down (row change)
    if (rCtrl.IsTrig(kiwi::EButton_Up)) {
        mKeyNo = kiwi::Max<s32>(0, mKeyNo - 3);
    } else if (rCtrl.IsTrig(kiwi::EButton_Down)) {
        mKeyNo = kiwi::Min<s32>(mKeys.Length() - 1, mKeyNo + 3);
    }

    // Now the cursor reflects the right one
    mKeys[mKeyNo].hover = true;

    // A button to press key
    if (rCtrl.IsTrig(kiwi::EButton_A)) {
        Key& rKey = mKeys[mKeyNo];

        if (rKey.pCallback != nullptr) {
            rKey.pCallback(rKey.code, rKey.pArg);
        }
    }
}

/**
 * @brief Standard draw pass
 */
void Keypad::UserDraw() const {
    f32 x = KEYPAD_X;
    f32 y = KEYPAD_Y;

    // Draw keys
    for (int i = 0; i < mKeys.Length(); i++) {
        const Key& rKey = mKeys[i];

        // Emphasize selected key
        f32 scale = rKey.hover ? 1.25f : 1.0f;
        kiwi::Color color = rKey.hover ? kiwi::Color::RED : kiwi::Color::WHITE;

        // kiwi::DebugPrint::PrintfOutline(x, y, 1.5f, true, color,
        //                                 kiwi::Color::GREY, rKey.code);

        // Next column
        x += KEY_SIZE;

        // Next row
        if (i > 0 && ((i + 1) % 3) == 0) {
            y -= KEY_SIZE;
            x = KEYPAD_X;
        }
    }

    // Draw buffer
    // kiwi::DebugPrint::PrintfOutline(0.0f, BUFFER_Y, 1.5f, true,
    //                                 kiwi::Color::WHITE, kiwi::Color::GREY,
    //                                 mBuffer);
}

/**
 * @brief Numeric key callback
 *
 * @param rCode Key code
 * @param pArg User argument (parent keypad)
 */
void Keypad::NumericKeyCallback(const kiwi::String& rCode, void* pArg) {
    ASSERT(pArg != nullptr);
    Keypad* p = static_cast<Keypad*>(pArg);

    // Append key code
    if (p->mBuffer.Length() + rCode.Length() <= CHARS_MAX) {
        p->mBuffer += rCode;
    }
}

/**
 * @brief Backspace key callback
 *
 * @param rCode Key code
 * @param pArg User argument (parent keypad)
 */
void Keypad::BackKeyCallback(const kiwi::String& rCode, void* pArg) {
#pragma unused(rCode)

    ASSERT(pArg != nullptr);
    Keypad* p = static_cast<Keypad*>(pArg);

    // Trim off one character
    if (!p->mBuffer.Empty()) {
        p->mBuffer = p->mBuffer.SubStr(0, p->mBuffer.Length() - 1);
    }
}

/**
 * @brief 'OK' key callback
 *
 * @param rCode Key code
 * @param pArg User argument (parent keypad)
 */
void Keypad::OkKeyCallback(const kiwi::String& rCode, void* pArg) {
#pragma unused(rCode)

    ASSERT(pArg != nullptr);
    Keypad* p = static_cast<Keypad*>(pArg);

    // Hand off to user callback
    if (p->mpOkCallback != nullptr) {
        p->mpOkCallback(p->mBuffer, p->mpOkCallbackArg);
    }
}

} // namespace BAH
