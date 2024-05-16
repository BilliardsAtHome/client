#include "core/Simulation.h"

#include <Pack/RPSystem.h>
#include <kokeshi.hpp>
#include <libkiwi.h>
#include <revolution/SC.h>

/**
 * Mod entrypoint
 */
void KokeshiMain() {
#ifndef NDEBUG
    // Setup libkiwi debugging utilities
    kiwi::Nw4rException::CreateInstance();
    kiwi::MapFile::CreateInstance();
    kiwi::MapFile::GetInstance().Open(kokeshi::scMapfilePath,
                                      kiwi::MapFile::ELinkType_Relocatable);
#endif

    kiwi::LibSO::Initialize();

    ASSERT_EX(SCGetAspectRatio() == SC_ASPECT_STD,
              "16:9 aspect ratio is not supported.\nPlease change to 4:3 in "
              "the console settings.");

    BAH::Simulation::CreateInstance();

    // Enter game loop
    RP_GET_INSTANCE(RPSysSystem)->mainLoop();
    // Main function should never return
    ASSERT(false);
}
KOKESHI_BY_PACK(KM_BRANCH(0x80183f04, KokeshiMain), // Wii Sports
                KM_BRANCH(0x80183b3c, KokeshiMain), // Wii Play
                KOKESHI_NOTIMPLEMENTED);            // Wii Sports Resort
