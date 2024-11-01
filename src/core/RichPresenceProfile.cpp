#include "core/RichPresenceProfile.h"

#include <libkiwi.h>

namespace BAH {

/**
 * @brief Discord Developer Portal application ID
 */
const kiwi::String RichPresenceProfile::APPLICATION_ID = "1301761992910176278";

/**
 * @brief Gets the Discord Developer Portal application ID
 */
kiwi::String RichPresenceProfile::GetAppID() const {
    return APPLICATION_ID;
}

/**
 * @brief Handles scene change event
 *
 * @param rClient Active rich presence client
 * @param scene Current scene ID
 */
void RichPresenceProfile::SceneCallback(kiwi::IRichPresenceClient& rClient,
                                        s32 scene) {
#pragma unused(scene)

    // Assume the scene is always Billiards
    rClient.SetStartTimeNow();
    rClient.SetDetails("Bruteforcing away!");
}

/**
 * @brief Handles periodic alarm event
 * @note The alarm period is 20 seconds.
 *
 * @param rClient Active rich presence client
 */
void RichPresenceProfile::AlarmCallback(kiwi::IRichPresenceClient& rClient) {
    // Pick a random table view image
    s32 choice = mRandom.NextU32(TABLE_IMAGE_NUM);
    kiwi::String name = kiwi::Format("table%03d", choice);
    rClient.SetLargeImageKey(name);
}

} // namespace BAH
