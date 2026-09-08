#include "patches.h"

#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/render_callback.h"
#include "game/audio/sound_manager.h"
#include "game/menu/race_setup/race_setup_menu.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/race/race_state.h"
#include "game/race/player/race_player_input.h"

RECOMP_PATCH void updateRaceSetupPlayerCountPrompt(MenuIntroActor *arg0) {
    RaceSetupMenuSubState *global;
    MenuIntroActor *actor;
    s32 globalState;
    s16 alpha;
    u8 state;
    s32 step;
    u32 stateCopy;

    global = &gRaceSetupMenuSubState;
    state = arg0->state;
    globalState = global->state;
    actor = arg0;
    stateCopy = state;
    if ((u32)stateCopy != globalState) {
        arg0->state = globalState;
        state = globalState;
        arg0->alpha = gRaceSetupMenuSubState.alpha;
    }
    step = 0x10;

    // @recomp Dismiss CHECK COMPLETE and its acknowledgement, including requests repeated by save panels.
    if ((state == 6) || (state == 7)) {
        actor->state = 8;
        actor->alpha = 0;
        global->state = 8;
        global->alpha = 0;
        return;
    }

    // @recomp Fade player-count options in place; retain state 5 for the parent's save-menu dispatch.
    if ((state == 4) || ((state == 3) && (gCurrentGameTask->callbackData1 == 2))) {
        actor->state = 4;
        actor->alpha -= 0x20;
        if (actor->alpha <= 0) {
            actor->alpha = 0;
            actor->state = 5;
        }
        global->state = actor->state;
        global->alpha = actor->alpha;
        if (actor->alpha != 0) {
            addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceSetupPlayerCountPrompt, actor);
        }
        return;
    }

    alpha = actor->alpha;
    if ((u32)alpha != 0x100) {
        if (state == 0) {
            actor->alpha = alpha + 0x20;
            alpha = actor->alpha;
            if ((u32)alpha == 0x100) {
                alpha = ((volatile MenuIntroActor *)actor)->alpha;
                actor->state = 1;
            }
        } else {
            actor->alpha = alpha - 0x30;
            alpha = actor->alpha;
            if (alpha <= 0) {
                actor->alpha = 0;
                alpha = actor->alpha;
            }
        }
    } else {
        switch (state) {
            case 1:
            case 6:
                alpha = ((volatile MenuIntroActor *)actor)->alpha;
                actor->timer = (actor->timer + 1) & 0xF;
                break;
            case 2:
                actor->y -= 0x10;
                if (actor->y == -0x5C) {
                    actor->child = createCallbackTask((CallbackTaskCallback)initRaceSetupOnePlayerOption, 0, 0x63);
                    enqueueSoundEffect(1, 0x32);
                    actor->state = 3;
                }
                alpha = actor->alpha;
                break;
            case 3:
                if (gCurrentGameTask->callbackData1 == 2) {
                    alpha = ((volatile MenuIntroActor *)actor)->alpha;
                    actor->state = 4;
                }
                break;
            case 4:
                actor->y += step;
                if (actor->y == -0x1C) {
                    actor->state = 5;
                }
                alpha = actor->alpha;
                break;
            case 5:
            case 7:
            case 8:
                break;
        }
    }

    if (alpha == 0) {
        actor->state = 8;
    }
    gRaceSetupMenuSubState.state = actor->state;
    gRaceSetupMenuSubState.alpha = actor->alpha;
    // @recomp Keep the checking state's dispatch signal without drawing its cosmetic page.
    if ((actor->state != 8) && (actor->state != 5)) {
        addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceSetupPlayerCountPrompt, (void *)actor);
    }
}
