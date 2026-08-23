#include "patches.h"

#include "game/engine/game_task_scheduler.h"
#include "game/engine/frame_render_task.h"

extern void submitFramebufferRenderTask(u8 frameIndex);

RECOMP_PATCH s32 updateFramebufferRenderScheduler(void) {
    u8 frameIndex;

    if (gFramebufferSubmissionCountdown[0] == 0) {
        if (gFramebufferSwapHold == 0) {
            frameIndex = gNextFramebufferRenderTaskIndex;
            if (gFrameRenderTasks[frameIndex].status == 0) {
                if ((s32)gPendingFramebufferSwapCount > 0) {
                    submitFramebufferRenderTask(frameIndex);
                    // @recomp Submit every tick instead of applying the multiplayer interval.
                    gFramebufferSubmissionCountdown[0] = 0;
                    gPendingFramebufferSwapCount--;
                    if (gNextFramebufferRenderTaskIndex != 0) {
                        gNextFramebufferRenderTaskIndex = 0;
                    } else {
                        gNextFramebufferRenderTaskIndex = 1;
                    }
                    goto return_one;
                }
                return 0;
            }
            return 0;
        }
        goto return_one;
    }
    gFramebufferSubmissionCountdown[0]--;

return_one:
    return 1;
}
