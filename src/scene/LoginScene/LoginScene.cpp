#include "scene/LoginScene/LoginScene.h"

#include "core/Simulation.h"

#include <Pack/RPAudio.h>
#include <libkiwi.h>

namespace BAH {

K_SCENE_DECL(LoginScene);

/**
 * @brief Setup scene
 */
void LoginScene::OnConfigure() {
    mKeypad.SetOkCallback(KeypadOkCallback, this);
}

/**
 * @brief Reset scene
 */
void LoginScene::OnReset() {
    mKeypad.Reset();
}

/**
 * @brief Update scene
 */
void LoginScene::OnCalculate() {
    mKeypad.Calculate();
}

/**
 * @brief Standard draw pass
 */
void LoginScene::OnUserDraw() {
    // clang-format off
    kiwi::DebugPrint::PrintfOutline(0.0f, 0.7f, 1.0f, true,
                                    kiwi::Color::WHITE, kiwi::Color::GREY,
                                    "Enter your unique ID:");

    kiwi::DebugPrint::PrintfOutline(0.0f, 0.6f, 1.0f, true,
                                    kiwi::Color::RED, kiwi::Color::GREY,
                                    "(from the bot)");

    kiwi::DebugPrint::PrintfOutline(0.0f, -0.6f, 0.8f, true,
                                    kiwi::Color::WHITE, kiwi::Color::GREY,
                                    "Enter the number with the D-Pad/A Button.");

    kiwi::DebugPrint::PrintfOutline(0.0f, -0.7f, 0.8f, true,
                                    kiwi::Color::YELLOW, kiwi::Color::GREY,
                                    "Use the /get-id bot command to check/get one.");
    // clang-format on

    mKeypad.UserDraw();
}

/**
 * @brief Keypad submit ('OK') callback
 *
 * @param rResult Keypad result sequence
 * @param pArg User argument
 */
void LoginScene::KeypadOkCallback(const kiwi::String& rResult, void* pArg) {
#pragma unused(pArg)

    // Work buffer (byte-aligned for NAND requirements)
    kiwi::WorkBufferArg arg;
    arg.size = sizeof(u32);
    kiwi::WorkBuffer buffer(arg);

    u32 uid = ksl::strtoul(rResult);
    Simulation::GetInstance().SetUniqueID(uid);

    // Write unique ID to buffer
    {
        kiwi::MemStream strm(buffer);
        ASSERT(strm.IsOpen());
        strm.Write_u32(uid);
    }

    // Save unique ID to the NAND
    {
        kiwi::NandStream strm("user.bin", kiwi::EOpenMode_Write);
        ASSERT(strm.IsOpen());
        strm.Write(buffer, buffer.AlignedSize());
    }

    // Disable audio (stop the "crash sound")
    // __OSStopAudioSystem();

    // Exit to billiards
    kiwi::SceneCreator::GetInstance().ChangeSceneAfterFade(
        kiwi::ESceneID_RPBilScene);
}

} // namespace BAH
