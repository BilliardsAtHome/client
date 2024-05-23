#include <cstring>
#include <libkiwi.h>
#include <revolution/EXI.h>
#include <revolution/OS.h>

namespace kiwi {
namespace detail {

/**
 * @brief Constructor
 */
Registers::Registers()
    : cr(0), xer(0), ctr(0), dsisr(0), dar(0), srr0(0), srr1(0), lr(0) {
    std::memset(gprs, 0, sizeof(gprs));
    std::memset(fprs, 0, sizeof(fprs));
}

/**
 * @brief Save CPU registers to this context
 *
 * @param ctx OS context
 * @param _dsisr DSISR register value
 * @param _dar DAR register value
 */
void Registers::Set(const OSContext& ctx, u32 _dsisr, u32 _dar) {
    cr = ctx.cr;
    xer = ctx.xer;
    ctr = ctx.ctr;
    srr0 = ctx.srr0;
    srr1 = ctx.srr1;
    lr = ctx.lr;

    dsisr = _dsisr;
    dar = _dar;

    std::memcpy(gprs, ctx.gprs, sizeof(gprs));
    std::memcpy(fprs, ctx.fprs, sizeof(fprs));
}

/**
 * @brief Load CPU registers from this context
 */
void Registers::Load() const {
    OSContext ctx;
    OSInitContext(&ctx, reinterpret_cast<void*>(srr0),
                  reinterpret_cast<void*>(gprs[1]));

    const OSContext* curr;
    curr = OSGetCurrentContext();
    K_ASSERT(curr != NULL);

    ctx.cr = cr;
    ctx.lr = lr;
    ctx.ctr = ctr;
    ctx.xer = xer;
    ctx.fpscr = curr->fpscr;
    ctx.srr1 = srr1;

    std::memcpy(ctx.gprs, gprs, sizeof(ctx.gprs));
    std::memcpy(ctx.fprs, fprs, sizeof(ctx.fprs));
    std::memcpy(ctx.gqrs, curr->gqrs, sizeof(ctx.gqrs));
    std::memcpy(ctx.psfs, curr->psfs, sizeof(ctx.psfs));

    OSLoadContext(&ctx);
}

} // namespace detail

K_DYNAMIC_SINGLETON_IMPL(Debugger);

/**
 * @brief Breakpoint exception handler
 */
void Debugger::BreakErrorHandler(u8 error, OSContext* ctx, u32 dsisr, u32 dar,
                                 ...) {
    Debugger& r = GetInstance();
    u32 srr1 = Mfsrr1();

    // Clear trace in context
    srr1 &= ~MSR_SE;
    r.mRegs.srr1 = srr1;

    // Clear trace, disable interrupts, etc.(?)
    srr1 &= ~0xFF00;
    srr1 |= MSR_FP;
    Mtsrr1(srr1);

    // Save exception context
    K_ASSERT(ctx != NULL);
    r.mRegs.Set(*ctx, dsisr, dar);

    // Clear HW breakpoint
    Mtiabr(0);
    Mtdabr(0);

    switch (error) {
    // Data breakpoint (vectored from exception handler)
    case OS_ERR_DSI:
        K_ASSERT_EX(dsisr & DSISR_DABR, "Real DSI exception???");
        break;
        ;

    // Step trace
    case OS_ERR_TRACE:
        break;
        ;
        ;

    // Instruction breakpoint
    case OS_ERR_IABR:
        break;
        ;
        ;
    }
}

/**
 * @brief Constructor
 */
Debugger::Debugger() : mStatus(EStatus_UnPaused) {
    OSSetErrorHandler(OS_ERR_TRACE, BreakErrorHandler);
    OSSetErrorHandler(OS_ERR_IABR, BreakErrorHandler);

    // DSI handler reserved for Nw4rException.
    // OSSetErrorHandler(OS_ERR_DSI, BreakErrorHandler);
}

/**
 * @brief Update debugger state
 */
void Debugger::Calculate() {
    NextCommand();
}

/**
 * @brief Read data from this device
 *
 * @param dst Destination buffer
 * @param size Bytes to read
 * @return Success
 */
bool Debugger::ExiReadN(void* dst, u32 size) {
    K_ASSERT(dst != NULL);
    K_ASSERT(size > 0);

    // Lock this device while we use it
    if (!EXILock(EXI_CHAN_0, EXI_DEV_EXT, NULL)) {
        return false;
    }

    // Setup channel parameters
    if (!EXISelect(EXI_CHAN_0, EXI_DEV_EXT, EXI_FREQ_32HZ)) {
        EXIUnlock(EXI_CHAN_0);
        return false;
    }

    bool success = true;

    // Prepare DMA
    u32 imm = 0xA0000000; // Read command
    success = success && EXIImm(EXI_CHAN_0, &imm, sizeof(u32), EXI_WRITE, NULL);
    success = success && EXISync(EXI_CHAN_0);

    // Read data
    success = success && EXIImmEx(EXI_CHAN_0, dst, size, EXI_READ);
    success = success && EXISync(EXI_CHAN_0);

    // Unlock this device when we are done
    success = success && EXIDeselect(EXI_CHAN_0);
    EXIUnlock(EXI_CHAN_0);
    return success;
}

/**
 * @brief Write data to this device
 *
 * @param src Source buffer
 * @param size Bytes to write
 * @return Success
 */
bool Debugger::ExiWriteN(const void* src, u32 size) {
    K_ASSERT(src != NULL);
    K_ASSERT(size > 0);

    // Lock this device while we use it
    if (!EXILock(EXI_CHAN_0, EXI_DEV_EXT, NULL)) {
        return false;
    }

    // Setup channel parameters
    if (!EXISelect(EXI_CHAN_0, EXI_DEV_EXT, EXI_FREQ_32HZ)) {
        EXIUnlock(EXI_CHAN_0);
        return false;
    }

    bool success = true;

    // Prepare DMA
    u32 imm = 0xB0000000; // Write command
    success = success && EXIImm(EXI_CHAN_0, &imm, sizeof(u32), EXI_WRITE, NULL);
    success = success && EXISync(EXI_CHAN_0);

    // Write data
    success = success &&
              EXIImmEx(EXI_CHAN_0, const_cast<void*>(src), size, EXI_WRITE);
    success = success && EXISync(EXI_CHAN_0);

    // Unlock this device when we are done
    success = success && EXIDeselect(EXI_CHAN_0);
    EXIUnlock(EXI_CHAN_0);
    return success;
}

/**
 * @brief Read and execute the next command from the USB Gecko
 */
void Debugger::NextCommand() {
#define HANDLE_CMD(x)                                                          \
    case ECommand_##x: Handler_##x(); break;

    u8 cmd = 0;
    if (!ExiRead(cmd) || cmd == 0) {
        return;
    }

    switch (cmd) {
        HANDLE_CMD(Write8);
        HANDLE_CMD(Write16);
        HANDLE_CMD(Write32);
        HANDLE_CMD(ReadN);
        HANDLE_CMD(Pause);
        HANDLE_CMD(UnPause);
        HANDLE_CMD(BreakPointData);
        HANDLE_CMD(BreakPointInsn);
        HANDLE_CMD(ReadContext);
        HANDLE_CMD(WriteContext);
        HANDLE_CMD(CancelBreakPoint);
        HANDLE_CMD(Step);
        HANDLE_CMD(GetStatus);
        HANDLE_CMD(BreakPointExact);
        HANDLE_CMD(GetVersion);

    default: K_ASSERT_EX(false, "Gecko command %d not supported", cmd); break;
    }

#undef HANDLE_CMD
}

/**
 * @brief Write a 8-bit value to memory
 */
void Debugger::Handler_Write8() {
    u8* dst = NULL;
    u32 value = 0;

    if (!ExiRead(dst) || !ExiRead(value)) {
        return;
    }

    K_ASSERT(dst != NULL);
    *dst = static_cast<u8>(value);
}

/**
 * @brief Write a 16-bit value to memory
 */
void Debugger::Handler_Write16() {
    u16* dst = NULL;
    u32 value = 0;

    if (!ExiRead(dst) || !ExiRead(value)) {
        return;
    }

    K_ASSERT(dst != NULL);
    *dst = static_cast<u16>(value);
}

/**
 * @brief Write a 32-bit value to memory
 */
void Debugger::Handler_Write32() {
    u32* dst = NULL;
    u32 value = 0;

    if (!ExiRead(dst) || !ExiRead(value)) {
        return;
    }

    K_ASSERT(dst != NULL);
    *dst = value;
}

/**
 * @brief Dump a range of memory
 */
void Debugger::Handler_ReadN() {
    const void* start = NULL;
    const void* end = NULL;

    if (!ExiRead(end) || !ExiRead(start)) {
        return;
    }

    K_ASSERT(end > start);
    ExiWriteN(start, PtrDistance(start, end));
}

/**
 * @brief Pause the program
 */
void Debugger::Handler_Pause() {
    mStatus = EStatus_Paused;
}

/**
 * @brief Un-pause the program
 */
void Debugger::Handler_UnPause() {
    mStatus = EStatus_UnPaused;
}

/**
 * @brief Set a PowerPC data breakpoint
 */
void Debugger::Handler_BreakPointData() {
    // Clear last "broken on" address
    if (mStatus != EStatus_BreakPoint) {
        mBP.last = 0;
    }

    mRegs.srr1 &= ~MSR_SE;

    // Load breakpoint info
    mBP.iabr = 0;
    ExiRead(mBP.dabr);
    mBP.align = 0;

    // Set HW breakpoint
    Mtiabr(mBP.iabr);
    Mtdabr(mBP.dabr);
}

/**
 * @brief Set a PowerPC instruction breakpoint
 */
void Debugger::Handler_BreakPointInsn() {
    // Clear last "broken on" address
    if (mStatus != EStatus_BreakPoint) {
        mBP.last = 0;
    }

    mRegs.srr1 &= ~MSR_SE;

    // Load breakpoint info
    ExiRead(mBP.iabr);
    mBP.dabr = 0;
    mBP.align = 0;

    // Set HW breakpoint
    Mtiabr(mBP.iabr);
    Mtdabr(mBP.dabr);
}

/**
 * @brief Read the breakpoint context
 */
void Debugger::Handler_ReadContext() {
    ExiWrite(mRegs);
}

/**
 * @brief Write the breakpoint context
 */
void Debugger::Handler_WriteContext() {
    ExiRead(mRegs);
}

/**
 * @brief Cancel the active breakpoint
 */
void Debugger::Handler_CancelBreakPoint() {
    // Clear breakpoint registers
    Mtdabr(0);
    Mtiabr(0);

    // Disable step trace
    mRegs.srr1 &= MSR_SE;
}

/**
 * @brief Write file contents to memory
 */
void Debugger::Handler_WriteFile() {
    K_ASSERT_EX(false, "Not yet implemented");

    // // Let PC know we are ready
    // ExiAck();

    // // Read memory region
    // const void* end = NULL;
    // const void* start = NULL;
    // if (!ExiRead(end) || !ExiRead(start)) {
    //     return;
    // }

    // K_ASSERT(end > start);
    // u32 size = PtrDistance(start, end);

    // // Read packet chunks
    // for (int i = 0; i < size / UPLOAD_PACKET_SIZE; i++) {
    //     ;
    // }
}

void Debugger::Handler_Step() {
    mRegs.srr1 &= ~MSR_SE;
}

/**
 * @brief Get the program execution status
 */
void Debugger::Handler_GetStatus() {
    ExiWrite<u8>(mStatus);
}

/**
 * @brief Set a Gecko breakpoint
 */
void Debugger::Handler_BreakPointExact() {
    // Clear last "broken on" address
    if (mStatus != EStatus_BreakPoint) {
        mBP.last = 0;
    }

    mRegs.srr1 &= ~MSR_SE;

    // Load breakpoint info
    mBP.iabr = 0;
    ExiRead(mBP.dabr);
    ExiRead(mBP.align);

    // Set HW breakpoint
    Mtiabr(mBP.iabr);
    Mtdabr(mBP.dabr);
}

/**
 * @brief Get the USB Gecko version
 */
void Debugger::Handler_GetVersion() {
    ExiWrite<u8>(EVersion_Rvl);
}

} // namespace kiwi