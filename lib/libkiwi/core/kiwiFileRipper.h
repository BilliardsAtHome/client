#ifndef LIBKIWI_CORE_FILE_RIPPER_H
#define LIBKIWI_CORE_FILE_RIPPER_H
#include <libkiwi/core/kiwiMemoryMgr.h>
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>

namespace kiwi {

// Forward declarations
class FileStream;
class MemStream;

/**
 * @brief File storage devices
 */
enum EStorage {
    EStorage_DVD,
    EStorage_NAND,
};

/**
 * @brief File ripping parameters
 */
struct FileRipperArg {
    // Where the file contents are stored.
    // Leave this as NULL to have the ripper allocate memory
    void* dst;

    // Where the file size is stored
    u32* size;

    // Memory region to use if the ripper must allocate its own destination
    // buffer
    kiwi::EMemory region;

    /**
     * @brief Constructor
     */
    FileRipperArg() : dst(NULL), size(NULL), region(kiwi::EMemory_MEM2) {}
};

/**
 * @brief File ripper/loader
 */
class FileRipper {
public:
    /**
     * @brief Rips a file's contents
     *
     * @param path Path to the file
     * @param where Storage device on which the file is located
     * @param arg Ripping parameters
     * @return File data (owned by you!)
     */
    static void* Rip(const String& path, EStorage where,
                     const FileRipperArg& arg = FileRipperArg());

    /**
     * @brief Rips a file's contents
     *
     * @param strm Stream to the file
     * @param arg Ripping parameters
     * @return File data (owned by you!)
     */
    static void* Rip(FileStream& strm,
                     const FileRipperArg& arg = FileRipperArg());

    /**
     * @brief Rips a file's contents and opens a stream to it
     *
     * @param path Path to the file
     * @param where Storage device on which the file is located
     * @return File stream
     */
    static MemStream Open(const String& path, EStorage where);
};

} // namespace kiwi

#endif