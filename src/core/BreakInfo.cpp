#include "BreakInfo.h"

#include "Simulation.h"

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
 * @brief Deserialize from stream
 *
 * @param strm Stream
 */
void BreakInfo::Read(kiwi::MemStream& strm) {
    seed = strm.Read_u32();
    kseed = strm.Read_u32();
    sunk = strm.Read_u32();
    off = strm.Read_u32();
    frame = strm.Read_u32();
    up = strm.Read_s32();
    left = strm.Read_s32();
    right = strm.Read_s32();
    pos.x = strm.Read_f32();
    pos.y = strm.Read_f32();
    power = strm.Read_f32();
    foul = strm.Read_s32();
    checksum = strm.Read_u32();

    u32 expected = CalcChecksum();
    K_WARN_EX(checksum != expected,
              "Checksum mismatch (expected %08X, got %08X)", expected,
              checksum);
}

/**
 * @brief Serialize to stream
 *
 * @param strm Stream
 */
void BreakInfo::Write(kiwi::MemStream& strm) const {
    strm.Write_u32(seed);
    strm.Write_u32(kseed);
    strm.Write_u32(sunk);
    strm.Write_u32(off);
    strm.Write_u32(frame);
    strm.Write_s32(up);
    strm.Write_s32(left);
    strm.Write_s32(right);
    strm.Write_f32(pos.x);
    strm.Write_f32(pos.y);
    strm.Write_f32(power);
    strm.Write_s32(foul);
    strm.Write_u32(CalcChecksum());
}

/**
 * @brief Calculate data checksum
 */
u32 BreakInfo::CalcChecksum() const {
    kiwi::Checksum crc;
    // Don't include 'checksum' member
    crc.Process(this, offsetof(BreakInfo, checksum));
    return crc.Result();
}

/**
 * @brief Compare break results
 *
 * @param other Comparison target
 */
bool BreakInfo::IsBetterThan(const BreakInfo& other) const {
    u32 myTotal = sunk + off;
    u32 otherTotal = other.sunk + other.off;

    // Compare total balls out of play
    if (myTotal != otherTotal) {
        return myTotal > otherTotal;
    }

    // Compare balls pocketed
    if (sunk != other.sunk) {
        return sunk > other.sunk;
    }

    // Compare foul
    if (foul != other.foul) {
        return foul == false;
    }

    // Compare frame count
    if (frame != other.frame) {
        return frame < other.frame;
    }

    // Tie, discard
    return false;
}

/**
 * @brief Log break result to the console
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
 * @brief Save break result to the NAND

 * @param name File name
 * @return Success
 */
void BreakInfo::Save(const char* name) const {
    // Work buffer (byte-aligned for NAND requirements)
    kiwi::WorkBuffer buffer(sizeof(BreakInfo));

    // Write break info to buffer
    {
        kiwi::MemStream strm(buffer);
        ASSERT(strm.IsOpen());
        Write(strm);
    }

    // Save break info to the NAND
    {
        kiwi::NandStream strm(kiwi::EOpenMode_Write);

        while (true) {
            // Attempt to open file
            bool success = strm.Open(name);
            if (success) {
                break;
            }

            // Failed? Try again in one second
            volatile s64 x = OSGetTime();
            while (OSGetTime() - x < OS_SEC_TO_TICKS(1)) {
                ;
            }
        }

        strm.Write(buffer, buffer.AlignedSize());
    }
}

/**
 * @brief Upload break result to the submission server
 */
void BreakInfo::Upload() const {
    kiwi::HttpRequest request("aspyn.gay");
    request.SetURI("/billiards/api");

    request.SetParameter("user", *Simulation::GetInstance().GetUniqueId());

    request.SetParameter("seed", kiwi::ToHexString(seed));
    request.SetParameter("kseed", kiwi::ToHexString(kseed));

    request.SetParameter("sunk", sunk);
    request.SetParameter("off", off);
    request.SetParameter("frame", frame);

    request.SetParameter("up", up);
    request.SetParameter("left", left);
    request.SetParameter("right", right);

    request.SetParameter("posx", kiwi::ToHexString(pos.x));
    request.SetParameter("posy", kiwi::ToHexString(pos.y));

    request.SetParameter("power", kiwi::ToHexString(power));
    request.SetParameter("foul", kiwi::ToHexString(foul));

    request.SetParameter("checksum", kiwi::ToHexString(CalcChecksum()));

    const kiwi::HttpResponse& resp = request.Send();
    if (resp.error != kiwi::EHttpErr_Success) {
        K_LOG_EX("HttpErr %d\n", resp.error);
    }
}

} // namespace BAH