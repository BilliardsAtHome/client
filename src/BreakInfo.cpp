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
      foul(false) {}

/**
 * @brief Deserialize from stream
 *
 * @param strm Stream
 */
void BreakInfo::Read(kiwi::IStream& strm) {
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
    foul = strm.Read_bool();

    // Checksum for integrity
    kiwi::Checksum crc;
    crc.Process(this, sizeof(BreakInfo));

    u32 expected = crc.Result();
    u32 got = strm.Read_u32();
    K_WARN_EX(expected != got, "Checksum mismatch (expected %08X, got %08X)",
              expected, got);
}

/**
 * @brief Serialize to stream
 *
 * @param strm Stream
 */
void BreakInfo::Write(kiwi::IStream& strm) const {
    // Checksum for integrity
    kiwi::Checksum crc;
    crc.Process(this, sizeof(BreakInfo));

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
    strm.Write_bool(foul);
    strm.Write_u32(crc.Result());
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
    LOG("BREAK = {");
    LOG_EX("    seed:\t%08X",        seed);
    LOG_EX("    kseed:\t%08X",       kseed);
    LOG_EX("    sunk:\t%d",          sunk);
    LOG_EX("    off:\t%d",           off);
    LOG_EX("    frame:\t%d",         frame);
    LOG_EX("    up:\t%d",            up);
    LOG_EX("    left:\t%d",          left);
    LOG_EX("    right:\t%d",         right);
    LOG_EX("    pos:\t{%08X, %08X}", kiwi::BitCast<u32>(pos.x), kiwi::BitCast<u32>(pos.y));
    LOG_EX("    power:\t%08X",       kiwi::BitCast<u32>(power));
    LOG_EX("    foul:\t%s",          foul ? "true" : "false");
    LOG("}");
    // clang-format on
}

/**
 * @brief Save break result to the NAND

 * @param name File name
 * @return Success
 */
void BreakInfo::Save(const char* name) const {
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

    Write(strm);
}

/**
 * @brief Upload break result to the submission server
 */
void BreakInfo::Upload() const {
    kiwi::HttpRequest request("127.0.0.1");
    request.SetURI("/billiards/api");

    request.SetParameter("user", Simulation::GetInstance().GetUserId());

    request.SetParameter("seed", kiwi::ToHexString(seed));
    request.SetParameter("kseed", kiwi::ToHexString(kseed));

    request.SetParameter("sunk", kiwi::ToString(sunk));
    request.SetParameter("off", kiwi::ToString(off));
    request.SetParameter("frame", kiwi::ToString(frame));

    request.SetParameter("up", kiwi::ToString(up));
    request.SetParameter("left", kiwi::ToString(left));
    request.SetParameter("right", kiwi::ToString(right));

    request.SetParameter("posx", kiwi::ToHexString(pos.x));
    request.SetParameter("posy", kiwi::ToHexString(pos.y));

    request.SetParameter("power", kiwi::ToHexString(power));
    request.SetParameter("foul", kiwi::ToHexString(foul));

    kiwi::Checksum crc;
    crc.Process(this, sizeof(BreakInfo));
    request.SetParameter("checksum", kiwi::ToHexString(crc.Result()));

    const kiwi::Optional<kiwi::HttpResponse>& resp = request.Send();
    K_ASSERT(resp);
}

} // namespace BAH