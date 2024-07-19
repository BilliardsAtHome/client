#ifndef RP_AUDIO_RP_TNK_SOUND_H
#define RP_AUDIO_RP_TNK_SOUND_H

//! @addtogroup rp_audio
//! @{

//! @file
//! @brief Sound IDs for the RPTnkScene sound archive

//! @}

namespace rp_Tnk_sound {

enum ESoundID {
    /* 0x00 */ RP_TNK_SE_RESULT_OPEN,
    /* 0x01 */ RP_TNK_SE_SHOOT_1P,
    /* 0x02 */ RP_TNK_SE_SHOOT_2P,
    /* 0x03 */ RP_TNK_SE_SHOOT_ENEMY1,
    /* 0x04 */ RP_TNK_SE_SHOOT_ENEMY2,
    /* 0x05 */ RP_TNK_SE_SHOOT_ENEMY3_LV,
    /* 0x06 */ RP_TNK_SE_SHOOT_ENEMY4,
    /* 0x07 */ RP_TNK_SE_SHOOT_ENEMY5,
    /* 0x08 */ RP_TNK_SE_SHOOT_ENEMY6,
    /* 0x09 */ RP_TNK_SE_SHOOT_ENEMY7_LV,
    /* 0x0A */ RP_TNK_SE_SHOOT_ENEMY8,
    /* 0x0B */ RP_TNK_SE_SHOOT_ENEMY9_LV,
    /* 0x0C */ RP_TNK_SE_SHOOT,
    /* 0x0D */ RP_TNK_SE_HIT_CSPK,
    /* 0x0E */ RP_TNK_SE_HIT,
    /* 0x0F */ RP_TNK_SE_REFLECT_SUPER,
    /* 0x10 */ RP_TNK_SE_REFLECT,
    /* 0x11 */ RP_TNK_SE_BROKEN,
    /* 0x12 */ RP_TNK_SE_TNK_DISAPPEAR,
    /* 0x13 */ RP_TNK_SE_JIRAI_SET_CSPK,
    /* 0x14 */ RP_TNK_SE_JIRAI_SET,
    /* 0x15 */ RP_TNK_SE_JIRAI_TIMER,
    /* 0x16 */ RP_TNK_SE_JIRAI_EXP,
    /* 0x17 */ RP_TNK_SE_JIRAI_EXP_MAE,
    /* 0x18 */ RP_TNK_SE_CATERPILLAR_1P,
    /* 0x19 */ RP_TNK_SE_CATERPILLAR_2P,
    /* 0x1A */ RP_TNK_SE_CATERPILLAR_SLOW,
    /* 0x1B */ RP_TNK_SE_CATERPILLAR_MID,
    /* 0x1C */ RP_TNK_SE_CATERPILLAR_SAME,
    /* 0x1D */ RP_TNK_SE_CATERPILLAR_FAST,
    /* 0x1E */ RP_TNK_SE_CATERPILLAR,
    /* 0x1F */ RP_TNK_SE_ENGINE,
    /* 0x20 */ RP_TNK_SE_IDLING,
    /* 0x21 */ RP_TNK_SE_MISSION_BONUS,
    /* 0x22 */ RP_TNK_SE_TNK1UP,
    /* 0x23 */ RP_TNK_SE_GAMESTART,
    /* 0x24 */ RP_TNK_SE_GET_TNKNUM,
    /* 0x25 */ RP_TNK_SE_GET_HIT,
    /* 0x26 */ RP_TNK_SE_GET_HIT_01,
    /* 0x27 */ RP_TNK_SE_GET_HIT_02,
    /* 0x28 */ RP_TNK_SE_GET_HIT_CSPK,
    /* 0x29 */ RP_TNK_SE_GET_HIT_01_CSPK,
    /* 0x2A */ RP_TNK_SE_GET_HIT_02_CSPK,
    /* 0x2B */ RP_TNK_SE_RESULT_TNK_TOTAL,
    /* 0x2C */ RP_TNK_SE_RESULT_TNK_LINE09,
    /* 0x2D */ RP_TNK_SE_RESULT_TNK_LINE08,
    /* 0x2E */ RP_TNK_SE_RESULT_TNK_LINE07,
    /* 0x2F */ RP_TNK_SE_RESULT_TNK_LINE06,
    /* 0x30 */ RP_TNK_SE_RESULT_TNK_LINE05,
    /* 0x31 */ RP_TNK_SE_RESULT_TNK_LINE04,
    /* 0x32 */ RP_TNK_SE_RESULT_TNK_LINE03,
    /* 0x33 */ RP_TNK_SE_RESULT_TNK_LINE02,
    /* 0x34 */ RP_TNK_SE_RESULT_TNK_LINE01,
    /* 0x35 */ RP_TNK_SE_INTRO_CATERPILLAR_00,
    /* 0x36 */ RP_TNK_SE_INTRO_CATERPILLAR_01,
    /* 0x37 */ RP_TNK_SE_INTRO_CATERPILLAR_02,
    /* 0x38 */ RP_TNK_SE_INTRO_CATERPILLAR_03,
    /* 0x39 */ RP_TNK_SE_INTRO_CATERPILLAR_04,
    /* 0x3A */ RP_TNK_SE_INTRO_CATERPILLAR_05,
    /* 0x3B */ RP_TNK_BGM_LOOP2,
    /* 0x3C */ RP_TNK_BGM_FILL2,
    /* 0x3D */ RP_TNK_BGM_RESULT2,
    /* 0x3E */ RP_TNK_BGM_COMP20,
    /* 0x3F */ RP_TNK_BGM_COMP100,
    /* 0x40 */ RP_TNK_BGM_MISS,
    /* 0x41 */ RP_TNK_BGM_PANPAKAPAN,
    /* 0x42 */ RP_COM_GAME_SE_PAUSE_01_CTSP,
    /* 0x43 */ RP_COM_GAME_SE_PAUSE_01_TVSP,
    /* 0x44 */ RP_COM_GAME_SE_PAUSE_01,
    /* 0x45 */ RP_COM_GAME_PARTY_SE_A_01_CTSP,
    /* 0x46 */ RP_COM_GAME_PARTY_SE_A_01_TVSP,
    /* 0x47 */ RP_COM_GAME_PARTY_SE_A_01,
    /* 0x48 */ RP_COM_GAME_PARTY_SE_Cursor_01,
    /* 0x49 */ RP_COM_GAME_PARTY_SE_LANK_01,
    /* 0x4A */ RP_COM_GAME_PARTY_SE_Win_Open_01,
    /* 0x4B */ RP_COM_GAME_PARTY_SE_Win_Close_01,
    /* 0x4C */ RP_COM_GAME_PARTY_SE_COUNT_TIME_01,
    /* 0x4D */ RP_COM_GAME_PARTY_SE_COUNT_TIME_02,
    /* 0x4E */ RP_COM_GAME_PARTY_SE_COUNT_TIME_03,
    /* 0x4F */ RP_COM_GAME_PARTY_SE_COUNT_TIME_x_01,
    /* 0x50 */ RP_COM_GAME_PARTY_SE_COUNT_01,
    /* 0x51 */ RP_COM_GAME_PARTY_SE_COUNT_02,
    /* 0x52 */ RP_COM_GAME_PARTY_SE_RESULT_KEY_A_01_CTSP,
    /* 0x53 */ RP_COM_GAME_PARTY_SE_RESULT_KEY_A_01_TVSP,
    /* 0x54 */ RP_COM_GAME_PARTY_SE_RESULT_KEY_A_01,
    /* 0x55 */ RP_COM_GAME_PARTY_SE_RESULT_KEY_A_02_CTSP,
    /* 0x56 */ RP_COM_GAME_PARTY_SE_RESULT_KEY_A_02_TVSP,
    /* 0x57 */ RP_COM_GAME_PARTY_SE_RESULT_KEY_A_02,
    /* 0x58 */ RP_COM_GAME_PARTY_SE_BUTTON_03,
    /* 0x59 */ RP_COM_GAME_PARTY_SE_BUTTON_05,
    /* 0x5A */ RP_COM_GAME_SE_PAUSE_KEY_A_01_CTSP,
    /* 0x5B */ RP_COM_GAME_SE_PAUSE_KEY_A_01_TVSP,
    /* 0x5C */ RP_COM_GAME_SE_PAUSE_KEY_A_01,
    /* 0x5D */ RP_COM_GAME_SE_PAUSE_KEY_A_02_CTSP,
    /* 0x5E */ RP_COM_GAME_SE_PAUSE_KEY_A_02_TVSP,
    /* 0x5F */ RP_COM_GAME_SE_PAUSE_KEY_A_02,
    /* 0x60 */ RP_COM_GAME_SE_PAUSE_KEY_A_03_CTSP,
    /* 0x61 */ RP_COM_GAME_SE_PAUSE_KEY_A_03_TVSP,
    /* 0x62 */ RP_COM_GAME_SE_PAUSE_KEY_A_03,
    /* 0x63 */ RP_COM_GAME_SE_PAUSE_Cursor_01,
    /* 0x64 */ RP_COM_GAME_SE_2P_WIN_LOSE_01,
    /* 0x65 */ RP_COM_BGM_GET_COPPER,
    /* 0x66 */ RP_COM_BGM_GET_SILVER,
    /* 0x67 */ RP_COM_BGM_GET_GOLD,
    /* 0x68 */ RP_COM_BGM_GET_PLATINA,
    /* 0x69 */ RP_COM_BGM_RENEW_RECORD2,
    /* 0x6A */ RP_COM_BGM_RENEW_RECORD,
    /* 0x6B */ RP_COMMON_SE_MESS_10,
    /* 0x6C */ RP_COMMON_SE_MESS_20,
    /* 0x6D */ RP_COMMON_SE_KEY_A_04_CTSP,
    /* 0x6E */ RP_COMMON_SE_KEY_A_04_TVSP,
    /* 0x6F */ RP_COMMON_SE_KEY_A_04,
    /* 0x70 */ RP_COMMON_SE_KEY_BACK_01_CTSP,
    /* 0x71 */ RP_COMMON_SE_KEY_BACK_01_TVSP,
    /* 0x72 */ RP_COMMON_SE_KEY_BACK_01,
    /* 0x73 */ RP_COMMON_SE_KEY_CURSOR_01
};

} // namespace rp_Tnk_sound

#endif
