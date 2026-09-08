#include "patches.h"

#include "game/demo/title_demo_race_intro.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/race_setup/race_setup_menu.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/race/player/race_player_input.h"
#include "game/race/race_state.h"

extern void recomp_set_game_player_count(u32 playerCount);

extern u8 gConnectedControllerCount;
extern u8 gMainMenuReturnFromRace;
extern s32 gMenuFlowState;
extern CallbackTask *D_8010ADE0;
extern CallbackTask *D_8010ADE4;
extern s16 gRaceSetupSavePanelInitialRects[4][2];

RECOMP_PATCH void initRaceSetupSaveMenu(void) {
    s32 i;
    s32 connectedControllerCount;
    RacePlayer *player;

    // @recomp Open or close player assignment after the chosen player count is final.
    recomp_set_game_player_count(gPlayerCount);

    for (i = 0; i < 4; i++) {
        gControllerPakStatusCodes[i] = 0;
        gMenuChoicePromptState[i] = 0;
        gControllerPakRetryCounts[i] = 0;
        gControllerPakOperationCounts[i] = 0;
    }

    gRaceSetupSavePanelCreateTimer = 0;
    do { i = 0; connectedControllerCount = gConnectedControllerCount; if (connectedControllerCount > 0) { player = gRacePlayers; do { player++; player[-1].menuState = 0; i++; } while (player < &gRacePlayers[connectedControllerCount]); i = 0; } do { initRaceSetupPlayerSaveData(i); i++; } while (i < 4); D_8010ADE0 = 0; D_8010ADE4 = 0; } while (0);
    D_8010ADE8 = 0;
    gMenuSelectionConfirmTimer = 0;
    gMenuFlowState = 0;
    gRaceRumbleEnabled = 0;

    for (i = 0; i < 4; i++) {
        gRaceSetupSavePanelRects[0][i] = gRaceSetupSavePanelInitialRects[i][0];
        gRaceSetupSavePanelRects[1][i] = gRaceSetupSavePanelInitialRects[i][1];
    }

    // @recomp Dismiss checking/check-complete pages for all players
    gRaceSetupMenuSubState.state = 8;
    gRaceSetupMenuSubState.alpha = 0;

    setCurrentGameTaskCallback(updateRaceSetupSaveMenu, 0);
    updateCallbackTasks();
}

RECOMP_PATCH void enterMainMenuFromRace(void) {
    // @recomp Return the frontend to single-player input when leaving a race.
    recomp_set_game_player_count(1);

    gMainMenuReturnFromRace = 1;
    setCurrentGameTaskCallback(initMainMenu, 0);
    createGameTask(4, initTitleDemoRaceIntro, 0x64);
    suspendGameTask(3);
}
