#include "patches.h"

#include "game/engine/callback_task_scheduler.h"
#include "game/engine/render_callback.h"
#include "game/menu/controller_pak/controller_pak_ui.h"

RECOMP_PATCH void updateControllerPakRumbleCheckPrompt(ControllerPakRumbleCheckPromptActor *arg0) {
    u8 state;
    u8 globalState;

    // @recomp Skip the initial and acknowledged DO NOT REMOVE RUMBLE PAK warning, preserving device-result pages.
    if (((gControllerPakRumbleCheckPromptTransition.state == 0) &&
         (gControllerPakRumbleCheckPromptTransition.selectedOption == 1)) ||
        (gControllerPakRumbleCheckPromptTransition.state == 3)) {
        arg0->state = 5;
        arg0->scale = 0;
        gControllerPakRumbleCheckPromptTransition.state = 5;
        removeCallbackTask(arg0);
        return;
    }

    state = arg0->state;
    if (state != (globalState = gControllerPakRumbleCheckPromptTransition.state)) {
        arg0->state = globalState;
        if (1) {}
        {}
        if (1) {}
        if (1) {}
        if (1) {}
        arg0->messageIndex = gControllerPakRumbleCheckPromptTransition.messageIndex;
        arg0->timer = 0;
        arg0->optionScale = 0x100; state = globalState;
    }

    switch (state) {
        case 0:
            arg0->scale += 0x28;
            if (arg0->scale >= 0x100) {
                arg0->scale = 0x100;
                if (gControllerPakRumbleCheckPromptTransition.selectedOption == 1) {
                    arg0->state = 3;
                } else {
                    arg0->state = 1;
                }
            }
            state = arg0->state;
            break;
        case 1:
            state = arg0->state;
            arg0->timer = (arg0->timer + 1) & 0xF;
            break;
        case 2:
            state = arg0->state;
            arg0->timer = 0;
            break;
        case 3:
            state = arg0->state;
            arg0->timer = (arg0->timer + 1) & 0xF;
            break;
        case 4:
            arg0->scale -= 0x28;
            if (arg0->scale <= 0) {
                arg0->scale = 0;
                arg0->state = 5;
            }
            state = arg0->state;
            break;
        case 7:
            state = arg0->state;
            arg0->timer = 0;
            break;
        case 8:
            state = arg0->state;
            arg0->timer = (arg0->timer + 1) & 0xF;
            break;
        case 9:
            if ((s32)arg0->timer < 0x10) {
                arg0->optionScale -= 9;
            } else {
                arg0->optionScale += 9;
            }
            state = arg0->state;
            arg0->timer = (arg0->timer + 1) & 0x1F;
            break;
        case 5:
        case 6:
            break;
    }

    gControllerPakRumbleCheckPromptState.state = state;
    if (arg0->state == 5) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawControllerPakRumbleCheckPrompt, (void *)arg0);
}
