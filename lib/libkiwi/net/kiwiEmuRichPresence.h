#ifndef LIBKIWI_NET_EMU_RICH_PRESENCE_H
#define LIBKIWI_NET_EMU_RICH_PRESENCE_H
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/net/kiwiIRichPresence.h>
#include <libkiwi/util/kiwiIosDevice.h>

namespace kiwi {

/**
 * @brief Rich presence implementation for Dolphin Emulator
 */
class EmuRichPresence : public IRichPresence {
public:
    /**
     * @brief Constructor
     *
     * @param client Client app ID
     */
    explicit EmuRichPresence(const String& client);

    /**
     * @brief Tests whether there is a connection established
     */
    virtual bool IsConnected() const {
        return mDevDolphin.IsOpen();
    }

    /**
     * @brief Retreive the current Unix epoch time (in seconds)
     */
    virtual u64 GetTimeNow() const;

    /**
     * @brief Update Discord client/app ID
     */
    virtual void UpdateClient() const;

    /**
     * @brief Update Discord presence status
     */
    virtual void UpdatePresence() const;

private:
    IosDevice mDevDolphin; // Handle to Dolphin device
};

} // namespace kiwi

#endif
