#ifndef BAH_CLIENT_BREAK_INFO_H
#define BAH_CLIENT_BREAK_INFO_H
#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Break shot configuration
 */
#pragma pack(push, 1)
struct BreakInfo {
    u32 seed;  //!< RPUtlRandom seed
    u32 kseed; //!< libkiwi seed

    u32 sunk;  //!< Balls sunk/pocketed
    u32 off;   //!< Balls hit off the table
    u32 frame; //!< Total frame count

    s32 up;    //!< Frames aimed up
    s32 left;  //!< Frames aimed left
    s32 right; //!< Frames aimed right

    EGG::Vector2f pos; //!< Cue position
    f32 power;         //!< Cue power

    bool foul;    //!< Foul status
    u32 checksum; //!< Data checksum

    //! Maximum attempts at NAND operations
    static const int NAND_RETRY_NUM = 10;
    //! Maximum attempts at Wi-Fi operations
    static const int WIFI_RETRY_NUM = 3;

    /**
     * @brief Constructor
     */
    BreakInfo();

    /**
     * @brief Deserializes break from stream
     *
     * @param rStrm Stream
     */
    void Read(kiwi::MemStream& rStrm);
    /**
     * @brief Serializes break to stream
     *
     * @param rStrm Stream
     */
    void Write(kiwi::MemStream& rStrm) const;
    /**
     * @brief Logs break to the console
     */
    void Log() const;

    /**
     * @brief Calculates data checksum
     */
    u32 CalcChecksum() const;

    /**
     * @brief Compares break results
     *
     * @param rOther Comparison target
     */
    bool IsBetterThan(const BreakInfo& rOther) const;

    /**
     * @brief Saves break result to the NAND
     *
     * @param rName File name
     * @return Success
     */
    void Save(const kiwi::String& rName) const;
    /**
     * @brief Uploads break result to the submission server
     *
     * @param rError HTTP error
     * @param rExError HTTP extended error
     * @param rStatus Response status code
     * @return Success
     */
    bool Upload(kiwi::EHttpErr& rError, s32& rExError,
                kiwi::EHttpStatus& rStatus) const;
};
#pragma pack(pop)

} // namespace BAH

#endif