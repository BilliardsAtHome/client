#ifndef BAH_CLIENT_BREAKINFO_H
#define BAH_CLIENT_BREAKINFO_H
#include <libkiwi.h>
#include <types.h>

namespace BAH {

/**
 * @brief Break shot configuration
 */
#pragma pack(push, 1)
struct BreakInfo {
    BreakInfo();
    void Read(kiwi::IStream& strm);
    void Write(kiwi::IStream& strm) const;

    bool IsBetterThan(const BreakInfo& other) const;
    void Log() const;
    void Save(const char* name) const;
    void Upload() const;

    u32 seed;
    u32 kseed;

    u32 sunk;
    u32 off;
    u32 frame;

    s32 up;
    s32 left;
    s32 right;

    EGG::Vector2f pos;
    f32 power;
    bool foul;
};
#pragma pack(pop)

} // namespace BAH

#endif