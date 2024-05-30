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
 * @brief Reload scene
 */
void LoginScene::OnReset() {
    mKeypad.Reset();
}

/**
 * @brief Scene logic
 */
void LoginScene::OnCalculate() {
    mKeypad.Calculate();
}

/**
 * @brief User-level draw
 */
void LoginScene::OnUserDraw() {
    kiwi::DebugPrint::PrintfOutline(0.0f, 0.7f, 1.0f, true, kiwi::Color::WHITE,
                                    kiwi::Color::GREY, "Enter your unique ID:");
    kiwi::DebugPrint::PrintfOutline(0.0f, 0.6f, 1.0f, true, kiwi::Color::RED,
                                    kiwi::Color::GREY, "(from the bot)");

    kiwi::DebugPrint::PrintfOutline(
        0.0f, -0.6f, 0.8f, true, kiwi::Color::WHITE, kiwi::Color::GREY,
        "Enter the number with the D-Pad/A Button.");
    kiwi::DebugPrint::PrintfOutline(
        0.0f, -0.7f, 0.8f, true, kiwi::Color::YELLOW, kiwi::Color::GREY,
        "Use the /get-id bot command to check/get one.");

    mKeypad.UserDraw();
}

/**
 * @brief Keypad OK keypress callback
 *
 * @param result Keypad result
 * @param arg Parent scene
 */
void LoginScene::KeypadOkCallback(const kiwi::String& result, void* arg) {
    LoginScene* self = static_cast<LoginScene*>(arg);
    ASSERT(self != NULL);

    // Work buffer (byte-aligned for NAND requirements)
    kiwi::WorkBuffer buffer(sizeof(u32));
    u32 uid = ksl::strtoul(result);

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

    Simulation::GetInstance().SetUniqueId(uid);

    // Disable audio (stop the "crash sound")
    __OSStopAudioSystem();

    // Exit to billiards
    kiwi::SceneCreator::GetInstance().ChangeSceneAfterFade(
        kiwi::ESceneID_RPBilScene);
}

} // namespace BAH
