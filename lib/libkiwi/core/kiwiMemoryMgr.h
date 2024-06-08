#ifndef LIBKIWI_CORE_MEMORY_MGR_H
#define LIBKIWI_CORE_MEMORY_MGR_H
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/util/kiwiStaticSingleton.h>

// Forward declarations
namespace EGG {
class Heap;
}

namespace kiwi {

/**
 * @brief Memory region
 */
enum EMemory {
    EMemory_MEM1, // 24MB RAM brought over from NGC. [0x80000000 - 0x817FFFFF]
    EMemory_MEM2, // 64MB RAM unique to RVL.         [0x90000000 - 0x93FFFFFF]

    EMemory_Max
};

/**
 * @brief Memory manager
 */
class MemoryMgr : public StaticSingleton<MemoryMgr> {
    friend class StaticSingleton<MemoryMgr>;

public:
    /**
     * @brief Allocates a block of memory
     *
     * @param size Block size
     * @param align Block alignment
     * @param memory Target memory region
     * @return void* Pointer to allocated block
     */
    void* Alloc(u32 size, s32 align, EMemory region);

    /**
     * @brief Frees a block of memory
     *
     * @param block Block
     */
    void Free(void* block);

    /**
     * @brief Gets total size of available heap memory
     *
     * @param memory Target memory region
     */
    u32 GetFreeSize(EMemory region);

    /**
     * @brief Tests whether an address points to an allocation from this manager
     *
     * @param addr Memory address
     */
    bool IsHeapMemory(const void* addr) const;

private:
    /**
     * @brief Constructor
     */
    MemoryMgr();
    /**
     * @brief Destructor
     */
    ~MemoryMgr();

private:
    EGG::Heap* mpHeapMEM1; // Heap in MEM1 region
    EGG::Heap* mpHeapMEM2; // Heap in MEM2 region

    // Initial size for both heaps
    static const u32 scHeapSize = OS_MEM_KB_TO_B(1024);
};

} // namespace kiwi

/**
 * @brief Allocates a block of memory
 *
 * @param size Block size
 * @return void* Pointer to allocated block
 */
void* operator new(std::size_t size);
/**
 * @brief Allocates a block of memory for an array
 *
 * @param size Block size
 * @return void* Pointer to allocated block
 */
void* operator new[](std::size_t size);

/**
 * @brief Allocates a block of memory
 *
 * @param size Block size
 * @param align Block address alignment
 * @return void* Pointer to allocated block
 */
void* operator new(std::size_t size, s32 align);
/**
 * @brief Allocates a block of memory for an array
 *
 * @param size Block size
 * @param align Block address alignment
 * @return void* Pointer to allocated block
 */
void* operator new[](std::size_t size, s32 align);

/**
 * @brief Allocates a block of memory
 *
 * @param size Block size
 * @param memory Target memory region
 * @return void* Pointer to allocated block
 */
void* operator new(std::size_t size, kiwi::EMemory memory);
/**
 * @brief Allocates a block of memory for an array
 *
 * @param size Block size
 * @param memory Target memory region
 * @return void* Pointer to allocated block
 */
void* operator new[](std::size_t size, kiwi::EMemory memory);

/**
 * @brief Allocates a block of memory
 *
 * @param size Block size
 * @param align Block address alignment
 * @param memory Target memory region
 * @return void* Pointer to allocated block
 */
void* operator new(std::size_t size, s32 align, kiwi::EMemory memory);
/**
 * @brief Allocates a block of memory for an array
 *
 * @param size Block size
 * @param align Block address alignment
 * @param memory Target memory region
 * @return void* Pointer to allocated block
 */
void* operator new[](std::size_t size, s32 align, kiwi::EMemory memory);

/**
 * @brief Frees a block of memory
 *
 * @param block Block
 */
void operator delete(void* block);
/**
 * @brief Frees a block of memory used by an array
 *
 * @param block Block
 */
void operator delete[](void* block);

#endif
