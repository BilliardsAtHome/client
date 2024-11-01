#include "core/BreakInfo.h"

#include "core/Simulation.h"

#include <libkiwi.h>

namespace BAH {

/**
 * @brief Constructor
 */
BreakInfo::BreakInfo()
    : seed(0),
      kseed(0),
      sunk(0),
      off(0),
      frame(0),
      up(0),
      left(0),
      right(0),
      pos(),
      power(0.0f),
      foul(false),
      checksum(0) {}

/**
 * @brief Deserializes break from stream
 *
 * @param rStrm Stream
 */
void BreakInfo::Read(kiwi::MemStream& rStrm) {
    seed = rStrm.Read_u32();
    kseed = rStrm.Read_u32();
    sunk = rStrm.Read_u32();
    off = rStrm.Read_u32();
    frame = rStrm.Read_u32();
    up = rStrm.Read_s32();
    left = rStrm.Read_s32();
    right = rStrm.Read_s32();
    pos.x = rStrm.Read_f32();
    pos.y = rStrm.Read_f32();
    power = rStrm.Read_f32();
    foul = rStrm.Read_s32();
    checksum = rStrm.Read_u32();

    u32 expected = CalcChecksum();
    K_WARN_EX(checksum != expected,
              "Checksum mismatch (expected %08X, got %08X)", expected,
              checksum);
}

/**
 * @brief Serializes break to stream
 *
 * @param rStrm Stream
 */
void BreakInfo::Write(kiwi::MemStream& rStrm) const {
    rStrm.Write_u32(seed);
    rStrm.Write_u32(kseed);
    rStrm.Write_u32(sunk);
    rStrm.Write_u32(off);
    rStrm.Write_u32(frame);
    rStrm.Write_s32(up);
    rStrm.Write_s32(left);
    rStrm.Write_s32(right);
    rStrm.Write_f32(pos.x);
    rStrm.Write_f32(pos.y);
    rStrm.Write_f32(power);
    rStrm.Write_s32(foul);
    rStrm.Write_u32(CalcChecksum());
}

/**
 * @brief Logs break to the console
 */
void BreakInfo::Log() const {
    // clang-format off
    LOG("BREAK = {\n");
    LOG_EX("    seed:\t%08X\n",        seed);
    LOG_EX("    kseed:\t%08X\n",       kseed);
    LOG_EX("    sunk:\t%d\n",          sunk);
    LOG_EX("    off:\t%d\n",           off);
    LOG_EX("    frame:\t%d\n",         frame);
    LOG_EX("    up:\t%d\n",            up);
    LOG_EX("    left:\t%d\n",          left);
    LOG_EX("    right:\t%d\n",         right);
    LOG_EX("    pos:\t{%08X, %08X}\n", kiwi::BitCast<u32>(pos.x), kiwi::BitCast<u32>(pos.y));
    LOG_EX("    power:\t%08X\n",       kiwi::BitCast<u32>(power));
    LOG_EX("    foul:\t%s\n",          foul ? "true" : "false");
    LOG("}\n");
    // clang-format on
}

/**
 * @brief Calculates data checksum
 */
u32 BreakInfo::CalcChecksum() const {
    kiwi::Checksum crc;

    // Don't include 'checksum' member
    crc.Process(this, offsetof(BreakInfo, checksum));
    return crc.Result();
}

/**
 * @brief Compares break results
 *
 * @param rOther Comparison target
 */
bool BreakInfo::IsBetterThan(const BreakInfo& rOther) const {
    u32 myTotal = sunk + off;
    u32 otherTotal = rOther.sunk + rOther.off;

    // Compare total balls out of play
    if (myTotal != otherTotal) {
        return myTotal > otherTotal;
    }

    // Compare balls pocketed
    if (sunk != rOther.sunk) {
        return sunk > rOther.sunk;
    }

    // Compare foul
    if (foul != rOther.foul) {
        return foul == false;
    }

    // Compare frame count
    if (frame != rOther.frame) {
        return frame < rOther.frame;
    }

    // Tie, discard
    return false;
}

/**
 * @brief Saves break result to the NAND
 *
 * @param rName File name
 * @return Success
 */
void BreakInfo::Save(const kiwi::String& rName) const {
    kiwi::WorkBufferArg arg;
    arg.size = sizeof(BreakInfo);
    kiwi::WorkBuffer buffer(arg);

    // Write break info to buffer
    {
        kiwi::MemStream strm(buffer);
        Write(strm);
    }

    // Save break info to the NAND
    {
        kiwi::NandStream strm(kiwi::EOpenMode_Write);

        for (int i = 0; i < NAND_RETRY_NUM; i++) {
            // Attempt to open file
            if (strm.Open(rName)) {
                break;
            }

            // Failed? Try again in one second
            volatile s64 x = OSGetTime();
            while (OSGetTime() - x < OS_SEC_TO_TICKS(1)) {
                ;
            }
        }

        ASSERT_EX(strm.IsOpen(), "NAND error");
        strm.Write(buffer, buffer.AlignedSize());
    }
}

/**
 * @brief Uploads break result to the submission server
 *
 * @param rError HTTP error
 * @param rExError HTTP extended error
 * @param rStatus Response status code
 * @return Success
 */
bool BreakInfo::Upload(kiwi::EHttpErr& rError, s32& rExError,
                       kiwi::EHttpStatus& rStatus) const {
    for (int i = 0; i < WIFI_RETRY_NUM; i++) {
        kiwi::HttpRequest request("127.0.0.1");
        request.SetURI("/billiards/api");

        // clang-format off
        request.SetParameter("user",     *Simulation::GetInstance().GetUniqueID());
        request.SetParameter("seed",     kiwi::ToHexString(seed));
        request.SetParameter("kseed",    kiwi::ToHexString(kseed));
        request.SetParameter("sunk",     sunk);
        request.SetParameter("off",      off);
        request.SetParameter("frame",    frame);
        request.SetParameter("up",       up);
        request.SetParameter("left",     left);
        request.SetParameter("right",    right);
        request.SetParameter("posx",     kiwi::ToHexString(pos.x));
        request.SetParameter("posy",     kiwi::ToHexString(pos.y));
        request.SetParameter("power",    kiwi::ToHexString(power));
        request.SetParameter("foul",     kiwi::ToHexString(foul));
        request.SetParameter("checksum", kiwi::ToHexString(CalcChecksum()));
        // clang-format on

        const kiwi::HttpResponse& rResp = request.Send();

        rError = rResp.error;
        rExError = rResp.exError;
        rStatus = rResp.status;

        if (rResp.error == kiwi::EHttpErr_Success &&
            rResp.status == kiwi::EHttpStatus_OK) {
            return true;
        }

        K_LOG_EX("try:%d err:%d ex:%d stat:%d\n", i, rResp.error, rResp.exError,
                 rResp.status);
    }

    return false;
}

} // namespace BAH