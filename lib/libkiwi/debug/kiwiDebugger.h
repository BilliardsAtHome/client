#ifndef LIBKIWI_DEBUG_DEBUGGER_H
#define LIBKIWI_DEBUG_DEBUGGER_H
#include <libkiwi/util/kiwiDynamicSingleton.h>
#include <types.h>

namespace kiwi {
namespace detail {

/**
 * @brief Register context
 */
struct Registers {
    /* 0x00 */ u32 cr;
    /* 0x04 */ u32 xer;
    /* 0x08 */ u32 ctr;
    /* 0x0C */ u32 dsisr;
    /* 0x10 */ u32 dar;
    /* 0x14 */ u32 srr0;
    /* 0x18 */ u32 srr1;
    /* 0x1C */ u32 lr;
    /* 0x20 */ u32 gprs[32];
    /* 0xA0 */ u64 fprs[32];

    /**
     * @brief Constructor
     */
    Registers();

    /**
     * @brief Save CPU registers to this context
     *
     * @param ctx OS context
     * @param _dsisr DSISR register value
     * @param _dar DAR register value
     */
    void Set(const OSContext& ctx, u32 _dsisr, u32 _dar);

    /**
     * @brief Load CPU registers from this context
     */
    void Load() const;
};

/**
 * @brief Breakpoint context
 */
struct Breakpoint {
    /* 0x00 */ u32 iabr;
    /* 0x04 */ u32 dabr;
    /* 0x08 */ u32 align;
    /* 0x0C */ u32 last;

    /**
     * @brief Constructor
     */
    Breakpoint() : iabr(0), dabr(0), align(0), last(0) {}
};

} // namespace detail

/**
 * @brief USB Gecko debugger support
 */
class Debugger : public DynamicSingleton<Debugger> {
    friend class DynamicSingleton<Debugger>;
    friend class Nw4rException;

public:
    /**
     * @brief Constructor
     */
    Debugger();
    /**
     * @brief Destructor
     */
    ~Debugger() {}

    /**
     * @brief Update debugger state
     */
    void Calculate();

    /**
     * @brief Tests whether program execution is paused
     */
    bool IsPaused() const {
        return mStatus == EStatus_Paused;
    }

private:
    static const u32 UPLOAD_PACKET_SIZE = 0xF80;

    enum EStatus {
        EStatus_Dummy,
        EStatus_UnPaused,
        EStatus_Paused,
        EStatus_BreakPoint
    };

    enum EVersion {
        EVersion_Rvl = 0x80,
        EVersion_Dol = 0x81,
    };

/**
 * @brief Define debugger command
 */
#define DEFINE_CMD(x, id)                                                      \
    static const u32 ECommand_##x = id;                                        \
    void Handler_##x();

    DEFINE_CMD(Write8, 0x01);
    DEFINE_CMD(Write16, 0x02);
    DEFINE_CMD(Write32, 0x03);
    DEFINE_CMD(ReadN, 0x04);
    DEFINE_CMD(Pause, 0x06);
    DEFINE_CMD(UnPause, 0x07);
    DEFINE_CMD(BreakPointData, 0x09);
    DEFINE_CMD(BreakPointInsn, 0x10);
    DEFINE_CMD(ReadContext, 0x2F);
    DEFINE_CMD(WriteContext, 0x30);
    DEFINE_CMD(CancelBreakPoint, 0x38);
    DEFINE_CMD(WriteFile, 0x41);
    DEFINE_CMD(Step, 0x44);
    DEFINE_CMD(GetStatus, 0x50);
    DEFINE_CMD(BreakPointExact, 0x89);
    DEFINE_CMD(GetVersion, 0x99);

#undef DEFINE_CMD

private:
    static void BreakErrorHandler(u8 error, OSContext* ctx, u32 dsisr, u32 dar,
                                  ...);

    template <typename T> static bool ExiRead(T& value) {
        return ExiReadN(&value, sizeof(T));
    }
    template <typename T> static bool ExiWrite(const T& value) {
        return ExiWriteN(&value, sizeof(T));
    }
    static bool ExiAck() {
        return ExiWrite<u8>(0xAA);
    }

    static bool ExiReadN(void* dst, u32 size);
    static bool ExiWriteN(const void* src, u32 size);

    void NextCommand();

private:
    // Program execution status
    EStatus mStatus;
    // Last execution context
    detail::Registers mRegs;
    // Active breakpoint
    detail::Breakpoint mBP;
};

} // namespace kiwi

#undef DEFINE_CMD

#endif
