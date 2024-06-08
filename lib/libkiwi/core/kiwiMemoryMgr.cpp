#include <egg/core.h>
#include <libkiwi.h>

namespace kiwi {
namespace {

/**
 * @brief Prints heap information
 *
 * @param name Heap name
 * @param heap Heap object
 */
void LogHeap(const char* name, EGG::Heap* heap) {
    if (heap == NULL) {
        K_LOG_EX("[%s] NULL ->\n", name);
        return;
    }

    K_LOG_EX("[%s] %p-> %.2fKB free\n", name, heap,
             OS_MEM_B_TO_KB(static_cast<f32>(heap->getAllocatableSize())));
}

/**
 * @brief Catches erroneous double-frees
 *
 * @param block Target of delete operation
 */
void CheckDoubleFree(const void* block) {
#ifndef NDEBUG
    // NULL delete is OK
    if (block == NULL) {
        return;
    }

    // Catch invalid pointers while we're here
    K_ASSERT(OSIsMEM1Region(block) || OSIsMEM2Region(block));

    // Sanity check, should always be ExpHeap
    MEMiHeapHead* handle = MEMFindContainHeap(block);
    K_ASSERT(handle != NULL);
    K_ASSERT(handle->magic == 'EXPH');

    // Check that the block is still marked as used
    MEMiExpHeapMBlock* memBlock = static_cast<MEMiExpHeapMBlock*>(
        AddToPtr(memBlock, -sizeof(MEMiExpHeapMBlock)));
    K_ASSERT_EX(memBlock->state == 'UD', "Double free!");
#endif
}

} // namespace

/**
 * @brief Constructor
 */
MemoryMgr::MemoryMgr() {
    // clang-format off
    mpHeapMEM1 = EGG::ExpHeap::create(scHeapSize, RPSysSystem::getSystemHeap(), 0);
    mpHeapMEM2 = EGG::ExpHeap::create(scHeapSize, RP_GET_INSTANCE(RPSysSystem)->getResourceHeap(), 0);

    LogHeap("RPSysSystem:System",   RPSysSystem::getSystemHeap());
    LogHeap("RPSysSystem:Resource", RP_GET_INSTANCE(RPSysSystem)->getResourceHeap());
    LogHeap("libkiwi:MEM1",         mpHeapMEM1);
    LogHeap("libkiwi:MEM2",         mpHeapMEM2);
    // clang-format on

    K_ASSERT(mpHeapMEM1 != NULL);
    K_ASSERT(mpHeapMEM2 != NULL);
    K_ASSERT(OSIsMEM1Region(mpHeapMEM1));
    K_ASSERT(OSIsMEM2Region(mpHeapMEM2));
}

/**
 * @brief Destructor
 */
MemoryMgr::~MemoryMgr() {
    delete mpHeapMEM1;
    delete mpHeapMEM2;
}

/**
 * @brief Allocates a block of memory
 *
 * @param size Block size
 * @param align Block alignment
 * @param memory Target memory region
 * @return void* Pointer to allocated block
 */
void* MemoryMgr::Alloc(u32 size, s32 align, EMemory memory) {
    K_ASSERT(memory < EMemory_Max);
    K_ASSERT(mpHeapMEM1 != NULL && mpHeapMEM2 != NULL);

    EGG::Heap* heap = memory == EMemory_MEM1 ? mpHeapMEM1 : mpHeapMEM2;
    void* block = heap->alloc(size, align);

    K_ASSERT_EX(block != NULL, "Out of memory (alloc %d)", size);
    K_ASSERT(memory == EMemory_MEM1 ? OSIsMEM1Region(block)
                                    : OSIsMEM2Region(block));

    return block;
}

/**
 * @brief Frees a block of memory
 *
 * @param block Block
 */
void MemoryMgr::Free(void* block) {
    K_ASSERT(mpHeapMEM1 != NULL && mpHeapMEM2 != NULL);
    CheckDoubleFree(block);
    EGG::Heap::free(block, NULL);
}

/**
 * @brief Gets total size of available heap memory
 *
 * @param memory Target memory region
 */
u32 MemoryMgr::GetFreeSize(EMemory memory) {
    K_ASSERT(memory < EMemory_Max);
    K_ASSERT(mpHeapMEM1 != NULL && mpHeapMEM2 != NULL);

    EGG::Heap* heap = memory == EMemory_MEM1 ? mpHeapMEM1 : mpHeapMEM2;
    return heap->getAllocatableSize();
}

/**
 * @brief Tests whether an address points to an allocation from this manager
 *
 * @param addr Memory address
 */
bool MemoryMgr::IsHeapMemory(const void* addr) const {
    K_ASSERT(mpHeapMEM1 != NULL && mpHeapMEM2 != NULL);

    // Check MEM1 heap
    if (addr >= mpHeapMEM1->getStartAddress() &&
        addr < mpHeapMEM1->getEndAddress()) {
        return true;
    }

    // Check MEM2 heap
    if (addr >= mpHeapMEM2->getStartAddress() &&
        addr < mpHeapMEM2->getEndAddress()) {
        return true;
    }

    return false;
}

} // namespace kiwi

/**
 * @brief Allocates a block of memory
 *
 * @param size Block size
 * @return void* Pointer to allocated block
 */
void* operator new(std::size_t size) {
    return kiwi::MemoryMgr::GetInstance().Alloc(size, 4, kiwi::EMemory_MEM1);
}
/**
 * @brief Allocates a block of memory for an array
 *
 * @param size Block size
 * @return void* Pointer to allocated block
 */
void* operator new[](std::size_t size) {
    return kiwi::MemoryMgr::GetInstance().Alloc(size, 4, kiwi::EMemory_MEM1);
}

/**
 * @brief Allocates a block of memory
 *
 * @param size Block size
 * @param align Block address alignment
 * @return void* Pointer to allocated block
 */
void* operator new(std::size_t size, s32 align) {
    return kiwi::MemoryMgr::GetInstance().Alloc(size, align,
                                                kiwi::EMemory_MEM1);
}
/**
 * @brief Allocates a block of memory for an array
 *
 * @param size Block size
 * @param align Block address alignment
 * @return void* Pointer to allocated block
 */
void* operator new[](std::size_t size, s32 align) {
    return kiwi::MemoryMgr::GetInstance().Alloc(size, align,
                                                kiwi::EMemory_MEM1);
}

/**
 * @brief Allocates a block of memory
 *
 * @param size Block size
 * @param memory Target memory region
 * @return void* Pointer to allocated block
 */
void* operator new(std::size_t size, kiwi::EMemory memory) {
    return kiwi::MemoryMgr::GetInstance().Alloc(size, 4, memory);
}
/**
 * @brief Allocates a block of memory for an array
 *
 * @param size Block size
 * @param memory Target memory region
 * @return void* Pointer to allocated block
 */
void* operator new[](std::size_t size, kiwi::EMemory memory) {
    return kiwi::MemoryMgr::GetInstance().Alloc(size, 4, memory);
}

/**
 * @brief Allocates a block of memory
 *
 * @param size Block size
 * @param align Block address alignment
 * @param memory Target memory region
 * @return void* Pointer to allocated block
 */
void* operator new(std::size_t size, s32 align, kiwi::EMemory memory) {
    return kiwi::MemoryMgr::GetInstance().Alloc(size, align, memory);
}
/**
 * @brief Allocates a block of memory for an array
 *
 * @param size Block size
 * @param align Block address alignment
 * @param memory Target memory region
 * @return void* Pointer to allocated block
 */
void* operator new[](std::size_t size, s32 align, kiwi::EMemory memory) {
    return kiwi::MemoryMgr::GetInstance().Alloc(size, align, memory);
}

/**
 * @brief Frees a block of memory
 *
 * @param block Block
 */
void operator delete(void* block) {
    kiwi::MemoryMgr::GetInstance().Free(block);
}
/**
 * @brief Frees a block of memory used by an array
 *
 * @param block Block
 */
void operator delete[](void* block) {
    kiwi::MemoryMgr::GetInstance().Free(block);
}
