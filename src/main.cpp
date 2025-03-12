#include "scene/SceneId.h"
#include <Pack/RPSystem.h>

#include <libkiwi.h>

#include <revolution/SC.h>

#include <kokeshi.hpp>

/**
 * Mod entrypoint
 */
void KokeshiMain() {
#ifndef NDEBUG
    // Setup libkiwi debugging utilities
    kiwi::Nw4rException::CreateInstance();
    kiwi::MapFile::CreateInstance();
    kiwi::MapFile::GetInstance().Open(kokeshi::MAPFILE_PATH,
                                      kiwi::MapFile::ELinkType_Relocatable);
#endif
    // Initialize socket system
    kiwi::LibSO::Initialize();

    ASSERT_EX(SCGetAspectRatio() == SC_ASPECT_STD,
              "16:9 aspect ratio is not supported.\nPlease change to 4:3 in "
              "the console settings.");

    // Change to bootup scene
    kiwi::SceneCreator::GetInstance().ChangeSceneAfterFade(
        BAH::ESceneID_SetupScene);

    // Enter game loop
    RP_GET_INSTANCE(RPSysSystem)->mainLoop();

    // Main function should never return
    ASSERT(false);
}
KOKESHI_BY_PACK(KM_CALL(0x80183b6c, KokeshiMain),  // Wii Sports
                KM_CALL(0x80183784, KokeshiMain),  // Wii Play
                KM_CALL(0x8022df10, KokeshiMain)); // Wii Sports Resort
