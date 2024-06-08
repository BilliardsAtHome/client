#include <cmath>
#include <libkiwi.h>
#include <revolution/OS.h>

namespace kiwi {
namespace {

/**
 * @brief Selects a random address in the specified range
 *
 * @param begin Beginning of range
 * @param end End of range
 */
void** GetRandomAddr(const void* begin, const void* end) {
    K_ASSERT(begin != NULL);
    K_ASSERT(end != NULL);
    K_ASSERT(end > begin);

    u32 size = GetOffsetFromPtr(begin, end);
    u32 offset = 0;

    do {
        // Make sure to align the address to 4 bytes
        offset = Random().NextU32(size);
        offset = ROUND_UP(offset, 4);
    }
    // Adjusting for alignment may take us out of the range
    while (offset >= size);

    return const_cast<void**>(AddToPtr<void*>(begin, offset));
}

} // namespace

K_DYNAMIC_SINGLETON_IMPL(GameCorruptor);

/**
 * @brief Constructor
 */
GameCorruptor::GameCorruptor()
    : mDomainFlag(ECorruptDomain_Mem2),
      mNumCorrupt(DEFAULT_NUM),
      mInterval(OS_SEC_TO_TICKS(DEFAULT_INTERVAL)) {}

/**
 * @brief Destructor
 */
GameCorruptor::~GameCorruptor() {
    OSCancelAlarm(&mAlarm);
}

/**
 * @brief Corruption alarm handler
 */
void GameCorruptor::AlarmHandler(OSAlarm* alarm, OSContext* ctx) {
#pragma unused(alarm)
#pragma unused(ctx)

    GetInstance().Corrupt();
}

/**
 * @brief Performs one corruption cycle
 */
void GameCorruptor::Corrupt() const {
    ECorruptDomain flag =
        static_cast<ECorruptDomain>(BitUtil::RandomBit(mDomainFlag));

    switch (flag) {
    case ECorruptDomain_DolCode:
        CorruptCode(GetDolTextStart(), GetDolTextEnd());
        break;

    case ECorruptDomain_DolData:
        // 50/50 between .data and .rodata
        if (Random().CoinFlip()) {
            CorruptData(GetDolDataStart(), GetDolDataEnd());
        } else {
            CorruptData(GetDolRodataStart(), GetDolRodataEnd());
        }
        break;

    case ECorruptDomain_Mem1:
        CorruptData(RP_GET_INSTANCE(RPSysSystem)->getRootHeapMem1());
        break;

    case ECorruptDomain_Mem2:
        CorruptData(RP_GET_INSTANCE(RPSysSystem)->getRootHeapMem2());
        break;

    case ECorruptDomain_Scene: CorruptData(EGG::Heap::getCurrentHeap()); break;

    default: K_ASSERT_EX(false, "Invalid corrupt flag"); break;
    }
}

/**
 * @brief Corrupts some code instructions in the specified range
 *
 * @param begin Beginning of range
 * @param end End of range
 */
void GameCorruptor::CorruptCode(const void* begin, const void* end) const {
    K_LOG_EX("CorruptCode %08X-%08X\n", begin, end);
}

/**
 * @brief Corrupts some pieces of data in the specified range
 *
 * @param begin Beginning of range
 * @param end End of range
 */
void GameCorruptor::CorruptData(const void* begin, const void* end) const {
    K_LOG_EX("CorruptData %08X-%08X\n", begin, end);

    for (int i = 0; i < mNumCorrupt;) {
        void** addr = GetRandomAddr(begin, end);

        // Don't corrupt libkiwi
        if (PtrUtil::IsLibKiwi(addr)) {
            continue;
        }

        // Try to avoid strings (may break filepaths)
        if (PtrUtil::IsString(addr)) {
            continue;
        }

        // Don't touch pointers
        if (PtrUtil::IsPointer(*addr) || *addr == 0) {
            continue;
        }
        if (PtrUtil::IsPtmf(addr)) {
            K_LOG_EX("PTMF? %08X\n", addr);
            continue;
        }

        // May corrupt a heap block tag
        if (PtrUtil::IsMBlockTag(addr) ||
            PtrUtil::IsMBlockTag(AddToPtr(addr, sizeof(u16)))) {
            K_LOG_EX("Heap tag? %08X\n", addr);
            continue;
        }

        // Corrupt float
        if (PtrUtil::IsFloat(addr)) {
            *reinterpret_cast<f32*>(addr) =
                Random().NextF32(10000000.0f) * Random().Sign();
        }
        // Corrupt integer
        else {
            *reinterpret_cast<u32*>(addr) = Random().NextU32();
        }

        i++;
    }
}

} // namespace kiwi