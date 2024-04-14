#include <libkiwi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Header for dynamically allocated arrays generated by CodeWarrior
 */
struct NewArrayHeader {
    /* 0x0 */ u32 size;
    /* 0x4 */ u32 count;
    /* 0x8 */ u8 padding[0x10 - 0x8];
};

/**
 * @brief Signed short argument for destructors generated by CodeWarrior
 */
enum DestructorType {
    DestructorType_AllBases = -1,   // Destroy all bases
    DestructorType_NonVirtualBases, // Destory all non-virtual bases
    DestructorType_All              // Destroy all bases and delete object
};

// Destructor function signature
typedef void (*Destructor)(void* obj, s16 type);

/**
 * @brief Destroy a dynamically allocated array
 *
 * @param array Array
 * @param dtor Destructor for each element
 */
void __destroy_new_array(u8* array, Destructor dtor) {
    if (array == NULL) {
        return;
    }

    // Header placed before contents
    const NewArrayHeader* header =
        reinterpret_cast<NewArrayHeader*>(array - sizeof(NewArrayHeader));

    if (dtor != NULL) {
        u32 size = header->size;
        u32 count = header->count;

        // Destroyed in reverse order
        u8* obj = array + (size * count);

        // Destroy all elements
        for (u32 i = 0; i < count; i++) {
            obj -= size;
            dtor(obj, DestructorType_AllBases);
        }
    }

    // Free base memory
    delete[] (array - sizeof(NewArrayHeader));
}

/**
 * @brief Convert s64 to f64
 */
asm void __cvt_sll_dbl() {
    80300794 94 21 ff f0     stwu       r1,-0x10(r1)
    80300798 54 65 00 01     rlwinm.    r5,r3,0x0,0x0,0x0
    8030079c 41 82 00 0c     beq        LAB_803007a8
    803007a0 20 84 00 00     subfic     r4,r4,0x0
    803007a4 7c 63 01 90     subfze     r3,r3
                            LAB_803007a8                                    XREF[1]:     8030079c(j)  
    803007a8 7c 67 23 79     or.        r7,r3,r4
    803007ac 38 c0 00 00     li         r6,0x0
    803007b0 41 82 00 80     beq        LAB_80300830
    803007b4 7c 67 00 34     cntlzw     r7,r3
    803007b8 7c 88 00 34     cntlzw     r8,r4
    803007bc 54 e9 d0 08     rlwinm     r9,r7,0x1a,0x0,0x4
    803007c0 7d 29 fe 70     srawi      r9,r9,0x1f
    803007c4 7d 29 40 38     and        r9,r9,r8
    803007c8 7c e7 4a 14     add        r7,r7,r9
    803007cc 21 07 00 20     subfic     r8,r7,0x20
    803007d0 31 27 ff e0     subic      r9,r7,0x20
    803007d4 7c 63 38 30     slw        r3,r3,r7
    803007d8 7c 8a 44 30     srw        r10,r4,r8
    803007dc 7c 63 53 78     or         r3,r3,r10
    803007e0 7c 8a 48 30     slw        r10,r4,r9
    803007e4 7c 63 53 78     or         r3,r3,r10
    803007e8 7c 84 38 30     slw        r4,r4,r7
    803007ec 7c c7 30 50     subf       r6,r7,r6
    803007f0 54 87 05 7e     rlwinm     r7,r4,0x0,0x15,0x1f
    803007f4 2c 07 04 00     cmpwi      r7,0x400
    803007f8 38 c6 04 3e     addi       r6,r6,0x43e
    803007fc 41 80 00 1c     blt        LAB_80300818
    80300800 41 81 00 0c     bgt        LAB_8030080c
    80300804 54 87 05 29     rlwinm.    r7,r4,0x0,0x14,0x14
    80300808 41 82 00 10     beq        LAB_80300818
                            LAB_8030080c                                    XREF[1]:     80300800(j)  
    8030080c 30 84 08 00     addic      r4,r4,0x800
    80300810 7c 63 01 94     addze      r3,r3
    80300814 7c c6 01 94     addze      r6,r6
                            LAB_80300818                                    XREF[2]:     803007fc(j), 80300808(j)  
    80300818 54 84 a8 3e     rlwinm     r4,r4,0x15,0x0,0x1f
    8030081c 50 64 a8 14     rlwimi     r4,r3,0x15,0x0,0xa
    80300820 54 63 ab 3e     rlwinm     r3,r3,0x15,0xc,0x1f
    80300824 54 c6 a0 16     rlwinm     r6,r6,0x14,0x0,0xb
    80300828 7c c3 1b 78     or         r3,r6,r3
    8030082c 7c a3 1b 78     or         r3,r5,r3
                            LAB_80300830                                    XREF[1]:     803007b0(j)  
    80300830 90 61 00 08     stw        r3,0x8(r1)
    80300834 90 81 00 0c     stw        r4,0xc(r1)
    80300838 c8 21 00 08     lfd        f1,0x8(r1)
    8030083c 38 21 00 10     addi       r1,r1,0x10
    80300840 4e 80 00 20     blr
}

/**
 * @brief Convert u64 to f64
 */
asm void __cvt_ull_dbl() {
    80059678 94 21 ff f0     stwu       r1,local_10(r1)
    8005967c 7c 67 23 79     or.        r7,r3,r4
    80059680 38 c0 00 00     li         r6,0x0
    80059684 41 82 00 7c     beq        LAB_80059700
    80059688 7c 67 00 34     cntlzw     r7,r3
    8005968c 7c 88 00 34     cntlzw     r8,r4
    80059690 54 e9 d0 08     rlwinm     r9,r7,0x1a,0x0,0x4
    80059694 7d 29 fe 70     srawi      r9,r9,0x1f
    80059698 7d 29 40 38     and        r9,r9,r8
    8005969c 7c e7 4a 14     add        r7,r7,r9
    800596a0 21 07 00 20     subfic     r8,r7,0x20
    800596a4 31 27 ff e0     subic      r9,r7,0x20
    800596a8 7c 63 38 30     slw        r3,r3,r7
    800596ac 7c 8a 44 30     srw        r10,r4,r8
    800596b0 7c 63 53 78     or         r3,r3,r10
    800596b4 7c 8a 48 30     slw        r10,r4,r9
    800596b8 7c 63 53 78     or         r3,r3,r10
    800596bc 7c 84 38 30     slw        r4,r4,r7
    800596c0 7c c7 30 50     subf       r6,r7,r6
    800596c4 54 87 05 7e     rlwinm     r7,r4,0x0,0x15,0x1f
    800596c8 2c 07 04 00     cmpwi      r7,0x400
    800596cc 38 c6 04 3e     addi       r6,r6,0x43e
    800596d0 41 80 00 1c     blt        LAB_800596ec
    800596d4 41 81 00 0c     bgt        LAB_800596e0
    800596d8 54 87 05 29     rlwinm.    r7,r4,0x0,0x14,0x14
    800596dc 41 82 00 10     beq        LAB_800596ec
                            LAB_800596e0                                    XREF[1]:     800596d4(j)  
    800596e0 30 84 08 00     addic      r4,r4,0x800
    800596e4 7c 63 01 94     addze      r3,r3
    800596e8 7c c6 01 94     addze      r6,r6
                            LAB_800596ec                                    XREF[2]:     800596d0(j), 800596dc(j)  
    800596ec 54 84 a8 3e     rlwinm     r4,r4,0x15,0x0,0x1f
    800596f0 50 64 a8 14     rlwimi     r4,r3,0x15,0x0,0xa
    800596f4 54 63 ab 3e     rlwinm     r3,r3,0x15,0xc,0x1f
    800596f8 54 c6 a0 16     rlwinm     r6,r6,0x14,0x0,0xb
    800596fc 7c c3 1b 78     or         r3,r6,r3
                            LAB_80059700                                    XREF[1]:     80059684(j)  
    80059700 90 61 00 08     stw        r3,local_8(r1)
    80059704 90 81 00 0c     stw        r4,local_8+0x4(r1)
    80059708 c8 21 00 08     lfd        f1,local_8(r1)
    8005970c 38 21 00 10     addi       r1,r1,0x10
    80059710 4e 80 00 20     blr
}

/**
 * @brief Convert s64 to f32
 */
asm void __cvt_sll_flt() {
    // clang-format off
    nofralloc

    stwu r1, -0x10(r1)
    rlwinm. r5, r3, 0, 0, 0
    beq lbl_80322DB4
    subfic r4, r4, 0
    subfze r3, r3
lbl_80322DB4:
    or. r7, r3, r4
    li r6, 0
    beq lbl_80322E3C
    cntlzw r7, r3
    cntlzw r8, r4
    rlwinm r9, r7, 0x1a, 0, 4
    srawi r9, r9, 0x1f
    and r9, r9, r8
    add r7, r7, r9
    subfic r8, r7, 0x20
    addic r9, r7, -32
    slw r3, r3, r7
    srw r10, r4, r8
    or r3, r3, r10
    slw r10, r4, r9
    or r3, r3, r10
    slw r4, r4, r7
    subf r6, r7, r6
    clrlwi r7, r4, 0x15
    cmpwi r7, 0x400
    addi r6, r6, 0x43e
    blt lbl_80322E24
    bgt lbl_80322E18
    rlwinm. r7, r4, 0, 0x14, 0x14
    beq lbl_80322E24
lbl_80322E18:
    addic r4, r4, 0x800
    addze r3, r3
    addze r6, r6
lbl_80322E24:
    rotlwi r4, r4, 0x15
    rlwimi r4, r3, 0x15, 0, 0xa
    rlwinm r3, r3, 0x15, 0xc, 0x1f
    slwi r6, r6, 0x14
    or r3, r6, r3
    or r3, r5, r3
lbl_80322E3C:
    stw r3, 8(r1)
    stw r4, 0xc(r1)
    lfd f1, 8(r1)
    frsp f1, f1
    addi r1, r1, 0x10
    blr
    // clang-format on
}

/**
 * @brief Convert u64 to f32
 */
asm void __cvt_ull_flt() {
    ;
}

/**
 * @brief Convert f64 to u64
 */
asm void __cvt_dbl_usll() {
    // clang-format off
    nofralloc

    stwu r1, -0x10(r1)
    stfd f1, 8(r1)
    lwz r3, 8(r1)
    lwz r4, 0xc(r1)
    rlwinm r5, r3, 0xc, 0x15, 0x1f
    cmplwi r5, 0x3ff
    bge lbl_80322E7C
    li r3, 0
    li r4, 0
    b lbl_80322F18
lbl_80322E7C:
    mr r6, r3
    clrlwi r3, r3, 0xc
    oris r3, r3, 0x10
    addi r5, r5, -1075
    cmpwi r5, 0
    bge lbl_80322EBC
    neg r5, r5
    subfic r8, r5, 0x20
    addic r9, r5, -32
    srw r4, r4, r5
    slw r10, r3, r8
    or r4, r4, r10
    srw r10, r3, r9
    or r4, r4, r10
    srw r3, r3, r5
    b lbl_80322F08
lbl_80322EBC:
    cmpwi r5, 0xa
    ble+ lbl_80322EE8
    rlwinm. r6, r6, 0, 0, 0
    beq lbl_80322ED8
    lis r3, 0x8000
    li r4, 0
    b lbl_80322F18
lbl_80322ED8:
    lis r3, 0x7FFFFFFF@h
    ori r3, r3, 0x7FFFFFFF@l
    li r4, -1
    b lbl_80322F18
lbl_80322EE8:
    subfic r8, r5, 0x20
    addic r9, r5, -32
    slw r3, r3, r5
    srw r10, r4, r8
    or r3, r3, r10
    slw r10, r4, r9
    or r3, r3, r10
    slw r4, r4, r5
lbl_80322F08:
    rlwinm. r6, r6, 0, 0, 0
    beq lbl_80322F18
    subfic r4, r4, 0
    subfze r3, r3
lbl_80322F18:
    addi r1, r1, 0x10
    blr
    // clang-format on
}

#ifdef __cplusplus
}
#endif
