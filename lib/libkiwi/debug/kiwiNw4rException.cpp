#include <cstdio>
#include <libkiwi.h>
#include <revolution/KPAD.h>
#include <revolution/VI.h>

namespace kiwi {

K_DYNAMIC_SINGLETON_IMPL(Nw4rException);

/**
 * @brief Constructor
 */
Nw4rException::Nw4rException()
    : mpUserCallback(DefaultCallback),
      mpUserCallbackArg(NULL),
      mpRenderMode(NULL) {
    Nw4rConsole::CreateInstance();

    // Create exception thread
    OSCreateThread(&mThread, ThreadFunc, NULL,
                   mThreadStack + sizeof(mThreadStack), sizeof(mThreadStack),
                   OS_PRIORITY_MIN, OS_THREAD_DETACHED);
    OSInitMessageQueue(&mMessageQueue, &mMessageBuffer, 1);
    OSResumeThread(&mThread);

    // Hook error handler
    OSSetErrorHandler(OS_ERR_DSI, ErrorHandler);
    OSSetErrorHandler(OS_ERR_ISI, ErrorHandler);
    OSSetErrorHandler(OS_ERR_ALIGNMENT, ErrorHandler);
    OSSetErrorHandler(OS_ERR_PROTECTION, ErrorHandler);
    OSSetErrorHandler(OS_ERR_FP_EXCEPTION, NULL);

    // Auto-detect render mode
    mpRenderMode = LibGX::GetDefaultRenderMode();
    K_WARN_EX(mpRenderMode == NULL, "No render mode!\n");
}

/**
 * @brief Sets user exception callback
 *
 * @param callback Exception callback function
 * @param arg Exception callback argument
 */
void Nw4rException::SetUserCallback(UserCallback callback, void* arg) {
    mpUserCallback = callback != NULL ? callback : DefaultCallback;
    mpUserCallbackArg = arg;
}

/**
 * @brief Writes text to the exception details
 *
 * @param fmt Format string
 * @param ... Format arguments
 */
void Nw4rException::Printf(const char* fmt, ...) {
    if (mpRenderMode == NULL) {
        return;
    }

    Nw4rDirectPrint::GetInstance().ChangeXfb(VIGetCurrentFrameBuffer(),
                                             mpRenderMode->fbWidth,
                                             mpRenderMode->xfbHeight);

    std::va_list list;
    va_start(list, fmt);
    Nw4rConsole::GetInstance().VPrintf(fmt, list);
    va_end(list);
}

/**
 * @brief Triggers an assertion error
 *
 * @param file Name of source file where assertion occurred
 * @param line Line in source file where assertion occurred
 * @param msg Assertion message/expression
 */
void Nw4rException::FailAssert(const char* file, int line, const char* msg) {
    mErrorInfo.assert.file = file;
    mErrorInfo.assert.line = line;
    mErrorInfo.assert.msg = msg;

    // Preserve stack pointer at time of assertion
    mErrorInfo.assert.sp = OSGetStackPointer();

    // Invalid error type to signal assertion
    ErrorHandler(EError_AssertFail, OSGetCurrentContext(), Mfdsisr(), Mfdar());
}

/**
 * @brief Exception thread main function
 */
void* Nw4rException::ThreadFunc(void* arg) {
#pragma unused(arg)
    Mtmsr(Mfmsr() & ~(MSR_FE0 | MSR_FE1));

    // Thread waits until error handler
    OSMessage msg;
    OSReceiveMessage(&GetInstance().mMessageQueue, &msg, OS_MSG_PERSISTENT);

    OSDisableInterrupts();
    VISetPreRetraceCallback(NULL);
    VISetPostRetraceCallback(NULL);
    VISetBlack(FALSE);
    VIFlush();

    GetInstance().DumpError();
    return NULL;
}

/**
 * @brief Handles errors (exception/assertion)
 *
 * @param error Exception type
 * @param ctx Exception context
 * @param dsisr Last DSISR value
 * @param dar Last DAR value
 */
void Nw4rException::ErrorHandler(u8 error, OSContext* ctx, u32 dsisr, u32 dar,
                                 ...) {
    Info& info = GetInstance().mErrorInfo;
    info.error = static_cast<EError>(error);
    info.ctx = ctx;
    info.dsisr = dsisr;
    info.dar = dar;
    info.msr = Mfmsr();

    // Vector data breakpoints to the debugger
    if (error == EError_DSI && (dsisr & DSISR_DABR)) {
        GlobalInstance<IDebugger>::Get().BreakCallback(error, ctx, dsisr, dar);
        return;
    }

    LibOS::FillFPUContext(ctx);
    OSSetErrorHandler(error, NULL);

    // Allow thread to continue
    OSSendMessage(&GetInstance().mMessageQueue, 0, OS_MSG_PERSISTENT);

    if (OSGetCurrentThread() == NULL) {
        VISetPreRetraceCallback(NULL);
        VISetPostRetraceCallback(NULL);
        Mtmsr(Mfmsr() | MSR_RI);
        GetInstance().DumpError();
    }

    while (OSEnableScheduler() > 0) {
        ;
    }

    OSYieldThread();
    OSDisableScheduler();
}

/**
 * @brief Allows controlling the console using the D-Pad
 *
 * @param info Exception info
 * @param arg Callback argument
 */
void Nw4rException::DefaultCallback(const Info& info, void* arg) {
#pragma unused(info)
#pragma unused(arg)
    Nw4rConsole::GetInstance().DrawDirect();

    while (true) {
        // Only re-draw when display is changed
        bool draw = false;

        for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
            KPADStatus ks;
            memset(&ks, 0, sizeof(KPADStatus));
            KPADRead(i, &ks, 1);

            // Vertical scroll
            if (ks.hold & KPAD_BTN_DUP) {
                Nw4rConsole::GetInstance().ScrollUp();
            } else if (ks.hold & KPAD_BTN_DDOWN) {
                Nw4rConsole::GetInstance().ScrollDown();
            }

            // Horizontal scroll
            if (ks.hold & KPAD_BTN_DLEFT) {
                Nw4rConsole::GetInstance().ScrollLeft();
            } else if (ks.hold & KPAD_BTN_DRIGHT) {
                Nw4rConsole::GetInstance().ScrollRight();
            }

            // Determine whether we need to draw the display again
            draw |= (ks.hold & (KPAD_BTN_DUP | KPAD_BTN_DDOWN | KPAD_BTN_DLEFT |
                                KPAD_BTN_DRIGHT)) != 0;
        }

        if (draw) {
            Nw4rConsole::GetInstance().DrawDirect();
        }

        // Wait 100ms before polling again
        s64 start = OSGetTime();
        while (OSGetTime() - start < OS_MSEC_TO_TICKS(100)) {
            ;
        }
    }
}

/**
 * @brief Dumps error information to the console
 */
void Nw4rException::DumpError() {
    OSDisableInterrupts();
    OSDisableScheduler();
    OSEnableInterrupts();

    // Acquire framebuffer
    Nw4rDirectPrint::GetInstance().SetupXfb();
    // Show console
    Nw4rConsole::GetInstance().SetVisible(true);

    // Dump information
    if (mErrorInfo.error == EError_AssertFail) {
        DumpAssert();
    } else {
        DumpException();
    }

    // Display information
    while (true) {
        Nw4rConsole::GetInstance().DrawDirect();

        if (mpUserCallback != NULL) {
            mpUserCallback(mErrorInfo, mpUserCallbackArg);
        }
    }

    // Spin lock
    volatile int x = 1;
    while (x) {
        ;
    }
}

/**
 * @brief Dumps exception information to the console
 */
void Nw4rException::DumpException() {
    // Basic information
    Printf("******** EXCEPTION OCCURRED! ********\n");
    Printf("Exception Type: %s\n", scExceptionNames[mErrorInfo.error]);
    Printf("SRR0: %08X\n", mErrorInfo.ctx->srr0);
    Printf("Symbol: ");
    PrintSymbol(reinterpret_cast<void*>(mErrorInfo.ctx->srr0));
    Printf("\n");
    Printf("\n");

    // Memory status
    PrintHeapInfo();

    // Build region/target
    PrintBuildInfo();

    // Context
    PrintGPR();

    // Stack trace
    PrintStack(scExceptionTraceDepth);

    PrintThankYouMsg();
}

/**
 * @brief Dumps assertion information to the console
 */
void Nw4rException::DumpAssert() {
    // Basic information
    Printf("******** ASSERTION FAILED! ********\n");
    Printf("%s\n", mErrorInfo.assert.msg);
    Printf("Source: %s(%d)\n", mErrorInfo.assert.file, mErrorInfo.assert.line);
    Printf("\n");

    // Memory status
    PrintHeapInfo();

    // Build region/target
    PrintBuildInfo();

    // Stack trace
    PrintStack(scAssertTraceDepth);

    PrintThankYouMsg();
}

/**
 * @brief Prints heap information to the screen
 */
void Nw4rException::PrintHeapInfo() {
    Printf("---Heap Info---\n");

    Printf("libkiwi (MEM1): %.2f KB free\n",
           OS_MEM_B_TO_KB(static_cast<f32>(
               MemoryMgr::GetInstance().GetFreeSize(EMemory_MEM1))));

    Printf("libkiwi (MEM2): %.2f KB free\n",
           OS_MEM_B_TO_KB(static_cast<f32>(
               MemoryMgr::GetInstance().GetFreeSize(EMemory_MEM2))));

    Printf("RPSysSystem: %.2f KB free\n",
           OS_MEM_B_TO_KB(static_cast<f32>(
               RPSysSystem::getSystemHeap()->getAllocatableSize())));

    Printf("\n");
}

/**
 * @brief Prints build information to the screen
 */
void Nw4rException::PrintBuildInfo() {
    Printf("---Build Info---\n");

    Printf("%s\n", GetBuildDate());
    Printf("%s (%s)\n", GetBuildPack(), GetBuildTarget());
    Printf("\n");
}

/**
 * @brief Prints GPR information to the screen
 */
void Nw4rException::PrintGPR() {
    Printf("---GPR Map---\n");
    Printf("-----------------------------------\n");

    for (int reg = 0; reg < 10; reg++) {
        Printf("R%02d:%08XH  R%02d:%08XH  R%02d:%08XH\n", reg,
               mErrorInfo.ctx->gprs[reg], reg + 11,
               mErrorInfo.ctx->gprs[reg + 11], reg + 22,
               mErrorInfo.ctx->gprs[reg + 22]);
    }

    Printf("R10:%08XH  R21:%08XH\n", mErrorInfo.ctx->gprs[10],
           mErrorInfo.ctx->gprs[21]);

    Printf("\n");
}

/**
 * @brief Prints stack trace information to the screen
 *
 * @param depth Stack trace max depth
 */
void Nw4rException::PrintStack(u32 depth) {
    /**
     * @brief Codewarrior stack frame
     */
    struct StackFrame {
        const StackFrame* next;
        const void* lr;
    };

    // Get stack pointer
    const StackFrame* frame = reinterpret_cast<const StackFrame*>(
        mErrorInfo.error == 0xFF
            ? mErrorInfo.assert.sp
            : reinterpret_cast<void*>(mErrorInfo.ctx->gprs[1]));

    Printf("---Stack Trace---\n");
    Printf("-----------------------------------\n");
    Printf("Address:   BackChain   LR save\n");

    for (int i = 0; i < depth; i++, frame = frame->next) {
        // Main thread root frame
        if (reinterpret_cast<u32>(frame) == 0xFFFFFFFF) {
            break;
        }

        // Sub thread root frame
        if (reinterpret_cast<u32>(frame) == 0) {
            break;
        }

        // Print stack frame info
        Printf("%08X:  %08X    ", frame, frame->next);
        PrintSymbol(frame->lr);
        Printf("\n");
    }

    Printf("\n");
}

/**
 * @brief Prints "thank you" message
 */
void Nw4rException::PrintThankYouMsg() {
    Printf("*************************************\n");
    Printf("\n");
    Printf("Thank you! You are a great debugger!\n");
}

/**
 * @brief Prints symbol information using the map file
 */
void Nw4rException::PrintSymbol(const void* addr) {
    // Symbol's offset from the start of game code
    std::ptrdiff_t textOffset = PtrDistance(GetModuleTextStart(), addr);

    // Symbol is from game (outside module)
    if (textOffset < 0 || textOffset >= GetModuleTextSize()) {
        // Print raw address
        Printf("%08X (game)", addr);
        return;
    }

    // Non-game symbol, see if a map file has been loaded
    if (!MapFile::GetInstance().IsAvailable()) {
        /**
         * If the map file is not available, we are in one of two
         * situations:
         * 1. The exception occurred before the map file could be read
         * 2. The map file does not exist
         *
         * Either way, we want to at least print the code's address
         * relocatable address (relative to the start of the module).
         */
        Printf("%08X (RELOC)", textOffset);
        return;
    }

    // Map file is available, let's use it
    const MapFile::Symbol* sym = MapFile::GetInstance().QueryTextSymbol(addr);

    /**
     * At this point we know the symbol is in module code, so the map
     * file should always return a result. However, to prevent the exception
     * handler from itself throwing an exception we do not assert this.
     */
    K_WARN_EX(sym == NULL,
              "Symbol missing(?) from module link map: reloc %08X\n",
              textOffset);

    // Symbol doesn't exist in map file
    if (sym == NULL) {
        // Print raw address
        Printf("%08X (MODULE)", addr);
        return;
    }

    // Offset into function where exception occurred
    u32 offset =
        sym->type == MapFile::ELinkType_Relocatable
            ? PtrDistance(AddToPtr(GetModuleTextStart(), sym->offset), addr)
            : PtrDistance(sym->addr, addr);

    // Print function name and instruction offset
    Printf("%s(+0x%04X)", sym->name, offset);
}

/**
 * @brief OS exception names
 */
const char* Nw4rException::scExceptionNames[OS_ERR_MAX] = {
    "System Reset",
    "Machine Check",
    "DSI",
    "ISI",
    "External Interrupt",
    "Alignment",
    "Program",
    "Floating Point",
    "Decrementer",
    "System Call",
    "Trace",
    "Performance Monitor",
    "Break Point",
    "System Interrupt",
    "Thermal Interrupt",
    "Protection",
    "Floating Point"};

} // namespace kiwi
