#include "patches.h"

#include "game/race/race_state.h"
#include "game/demo/title_demo_race_intro.h"
#include "game/audio/sound_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/camera/race_camera.h"
#include "game/race/player/race_player_update.h"
#include "game/engine/viewport_manager.h"
#include "game/race/player/race_player_input.h"
#include "game/menu/renderer/menu_screen_effects.h"

extern s16 gMenuFadeAlpha;
extern u8 gMenuFadeOverlayActive;
extern u8 gRaceUpdatePaused;
extern u8 gFramebufferSwapHold;

#define RACE_PLAYER_REPLAY_SNAPSHOT(index) (((RacePlayerReplaySnapshot *)gRacePlayers)[index])

RECOMP_PATCH void updateTitleDemoRaceIntro(void) {
    s32 fadeStep;
    union {
        s32 value;
        u8 padding[12];
    } previousPause;
    s32 cameraIndex;
    s32 fadeDelay;
    s32 nextViewportHeight;
    u32 i;
    // @recomp Keep byte writes volatile so clang cannot replace the loops with memcpy.
    volatile u8 *destination;

    previousPause.value = gRaceUpdatePaused;
    configureViewport(0, 0xA0, 0x78, 0x120, (u8) gTitleDemoRaceIntroViewportHeight, 0x140, 0xF0, 1.333333373f);
    {
        u8 *viewportHeight = (u8 *)&gTitleDemoRaceIntroViewportHeight;

        // @recomp Open the title-demo curtain to the full viewport height.
        if (*viewportHeight != 0xD0) {
            *viewportHeight += 0x10;
            // @recomp Create the start prompt when the full-height curtain finishes opening.
            if (*viewportHeight == 0xD0) {
                createCallbackTask((CallbackTaskCallback)updateTitleScreenStartPrompt, 0, 0x64);
            }
        }
    }
    do { fadeStep = gCurrentGameTask->callbackData1; if (fadeStep == gTitleDemoReplaySegmentFrames[gCurrentGameTask->callbackData2]) { destination = RACE_PLAYER_REPLAY_SNAPSHOT(0).bytes; i = 0; do { destination[i] = gTitleDemoReplayPlayerSnapshots[0][gCurrentGameTask->callbackData2].bytes[i]; i++; } while (i < sizeof(RacePlayerReplaySnapshot)); destination = RACE_PLAYER_REPLAY_SNAPSHOT(1).bytes; i = 0; do { destination[i] = gTitleDemoReplayPlayerSnapshots[1][gCurrentGameTask->callbackData2].bytes[i]; i++; if (1) { } } while (i < sizeof(RacePlayerReplaySnapshot)); destination = RACE_PLAYER_REPLAY_SNAPSHOT(2).bytes; i = 0; do { destination[i] = gTitleDemoReplayPlayerSnapshots[2][gCurrentGameTask->callbackData2].bytes[i]; i++; } while (i < sizeof(RacePlayerReplaySnapshot)); destination = RACE_PLAYER_REPLAY_SNAPSHOT(3).bytes; i = 0; while (i < sizeof(RacePlayerReplaySnapshot)) { destination[i] = gTitleDemoReplayPlayerSnapshots[3][gCurrentGameTask->callbackData2].bytes[i]; i++; } gCurrentGameTask->callbackData2++; fadeStep = gCurrentGameTask->callbackData1; } cameraIndex = gCurrentGameTask->callbackData3; if (fadeStep == gTitleDemoCameraModeFrames[cameraIndex]) { setRaceCameraMode(0, gTitleDemoCameraModes[cameraIndex]); gCurrentGameTask->callbackData3++; gRaceUpdatePaused = 1; } updateRacePlayers(); updateCallbackTasksWithMinPriority(0x63); updateRacePlayersPostUpdate(); updateRemainingCallbackTasks(); gRaceUpdatePaused = previousPause.value; } while (0);
    updateRaceCameras();
    gCurrentGameTask->callbackData1++;
    fadeDelay = gCurrentGameTask->callbackData0;
    if (fadeDelay != 0) {
        gCurrentGameTask->callbackData0 = fadeDelay - 1;
    }
    if (gPlayerInputPressed[0] & START_BUTTON) {
        if ((u8) gTitleDemoRaceIntroFadeStep == 0) {
            gTitleDemoRaceIntroFadeStep = 0x10;
        }
        requestMusicSequenceStop(0x20);
    }
    if (gCurrentGameTask->callbackData0 < 0x41) {
        if ((u8) gTitleDemoRaceIntroFadeStep == 0) {
            gTitleDemoRaceIntroFadeStep = 4;
        }
        requestMusicSequenceStop(0x82);
    }
    if ((u8) gTitleDemoRaceIntroFadeStep != 0) {
        gMenuFadeOverlayActive = 1;
        gMenuFadeAlpha += (u8)gTitleDemoRaceIntroFadeStep;
    }
    if (gMenuFadeAlpha >= 0xFF) {
        gMenuFadeAlpha = 0xFF;
        gFramebufferSwapHold = 1;
        setCurrentGameTaskCallback(finishTitleDemoRaceIntro, 0);
    }
}
