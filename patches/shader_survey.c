#include "patches.h"
#include "sys.h"
#include "sf64audio_external.h"

// The shader survey: lets a script start any level, or switch to any game state, by itself, to record the shaders
// each one draws for the page's prebuilt list (src/rt64_shaders.txt, made by tools/record_shaders.sh) without anyone
// playing. It does nothing unless the script asks.
//
// The script finds this struct in RDRAM by its two magic words and writes a game state into `state` (GSTATE_PLAY, 7,
// with a level in `level` and its phase in `phase` first: 1 is Meteo's and Sector X's warp zone); `taken` counts the
// requests taken, so it knows one was. Once a frame, before Game_Update, the request is handed to the game's own state
// change (gNextGameState, as a level's end or Area 6's warp does), with the level's audio spec set as the Lylat map
// sets it when a level starts (the ending's as the Andross fight's end does). While `invincible` is set the player's
// shields are kept full and the lives at 9, so a level plays on for as long as the script waits.

struct ShaderSurvey {
    u32 magic[2];
    volatile u32 taken;
    volatile s32 state;
    volatile s32 level;
    volatile s32 phase;
    volatile s32 invincible;
};

struct ShaderSurvey shader_survey = { { 0x53484452, 0x53525659 }, 0, -1, -1, 0, 0 }; // "SHDR" "SRVY"

static void shader_survey_set_audio_spec(s32 level) {
    // Map_LevelStart_AudioSpecSetup's (fox_map.c), which is in the map's overlay.
    static const u8 specs[] = {
        [LEVEL_CORNERIA] = AUDIOSPEC_CO, [LEVEL_METEO] = AUDIOSPEC_ME,   [LEVEL_TITANIA] = AUDIOSPEC_TI,
        [LEVEL_AQUAS] = AUDIOSPEC_AQ,    [LEVEL_BOLSE] = AUDIOSPEC_BO,   [LEVEL_KATINA] = AUDIOSPEC_KA,
        [LEVEL_AREA_6] = AUDIOSPEC_A6,   [LEVEL_SECTOR_Z] = AUDIOSPEC_SZ, [LEVEL_FORTUNA] = AUDIOSPEC_FO,
        [LEVEL_SECTOR_X] = AUDIOSPEC_SX, [LEVEL_MACBETH] = AUDIOSPEC_MA, [LEVEL_ZONESS] = AUDIOSPEC_ZO,
        [LEVEL_SECTOR_Y] = AUDIOSPEC_SY,
    };

    if (level == LEVEL_SOLAR) {
        AUDIO_SET_SPEC(SFX_LAYOUT_SO, AUDIOSPEC_SO);
    } else if ((level >= 0) && (level < (s32) ARRAY_COUNT(specs)) && ((specs[level] != 0) || (level == LEVEL_CORNERIA))) {
        AUDIO_SET_SPEC(SFX_LAYOUT_DEFAULT, specs[level]);
    }
}

void shader_survey_update(void) {
    if (shader_survey.state >= 0) {
        s32 state = shader_survey.state;

        shader_survey.state = -1;
        if (state == GSTATE_PLAY) {
            shader_survey_set_audio_spec(shader_survey.level);
            gNextLevel = shader_survey.level;
            gNextLevelPhase = shader_survey.phase;
        } else if (state == GSTATE_ENDING) {
            // As the Andross fight's end starts it (fox_andross.c).
            D_ending_80196D00 = 0;
            AUDIO_SET_SPEC(SFX_LAYOUT_DEFAULT, AUDIOSPEC_ENDING);
        }
        gNextGameState = state;
        shader_survey.taken++;
    }

    if (shader_survey.invincible && (gGameState == GSTATE_PLAY) && (gPlayer != NULL)) {
        gPlayer[0].shields = 255;
        gLifeCount[0] = 9;
    }
}
