#ifndef BAH_CLIENT_CORE_RICH_PRESENCE_PROFILE_H
#define BAH_CLIENT_CORE_RICH_PRESENCE_PROFILE_H
#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Billiards@home rich presence profile
 */
class RichPresenceProfile : public kiwi::IRichPresenceProfile {
public:
    /**
     * @brief Gets the Discord Developer Portal application ID
     */
    virtual kiwi::String GetAppID() const;

    /**
     * @brief Handles scene change event
     *
     * @param rClient Active rich presence client
     * @param scene Current scene ID
     */
    virtual void SceneCallback(kiwi::IRichPresenceClient& rClient, s32 scene);

    /**
     * @brief Handles periodic alarm event
     * @note The alarm period is 20 seconds.
     *
     * @param rClient Active rich presence client
     */
    virtual void AlarmCallback(kiwi::IRichPresenceClient& rClient);

private:
    //! Discord Developer Portal application ID
    static const kiwi::String APPLICATION_ID;

    //! Number of table view images
    static const int TABLE_IMAGE_NUM = 6;

private:
    //! Picture randomizer
    kiwi::Random mRandom;
};

} // namespace BAH

#endif
