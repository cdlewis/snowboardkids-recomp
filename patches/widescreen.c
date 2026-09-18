#include "patches.h"
#include "game/ending/ending_credits_flow.h"
#include "game/engine/viewport_manager.h"
#include "PR/gu.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/camera/race_camera.h"
#include "game/race/player/race_player_input.h"
#include "game/race/flow/race_flow.h"
#include "race_split_screen.h"
#include "podium_scene.h"
#include "training_viewport.h"
#include "game/menu/main_menu/training_course_race_flow.h"

extern s32 recomp_get_vertical_2p_split_screen_enabled(void);
static s32 sVerticalTwoPlayerSplit;

s32 raceUsesVerticalTwoPlayerSplit(void) {
    return sVerticalTwoPlayerSplit && gPlayerCount == 2 && gRaceCameras[0].initialized.value != 0;
}

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define VIEWPORT_EDGE_SNAP 16

static f32 sTrainingViewportTransitionProgress = -1.0f;

f32 trainingViewportTransitionScale(s32 index) {
    if (index != 0 || sTrainingViewportTransitionProgress < 0.0f) {
        return 0.0f;
    }
    return 1.0f + (recomp_get_target_aspect_ratio(4.0f / 3.0f) / (4.0f / 3.0f) - 1.0f) *
                      sTrainingViewportTransitionProgress;
}

static f32 configureTrainingViewportTransition(s32 index) {
    if (index == 0 && gCurrentGameTask != NULL) {
        if (gCurrentGameTask->callbacks[0] == initTrainingCourseRace) {
            return 0.0f;
        }
        if (gCurrentGameTask->callbacks[0] == zoomTrainingCourseRaceViewport &&
            gCurrentGameTask->callbackData0 > 0 && gCurrentGameTask->callbackData0 <= 16) {
            return gCurrentGameTask->callbackData0 / 16.0f;
        }
    }
    return -1.0f;
}

static f32 configureRaceSplitDirection(s32 index, s32 *x, s32 *y, u16 *width, u16 *height,
                                     u16 *scaleX, u16 *scaleY, f32 *aspect) {
    s32 startingRace = gCurrentGameTask != NULL &&
                      gCurrentGameTask->callbacks[0] == fadeInRaceGameplayViewports;
    s32 oldX = *x;
    u16 oldWidth = *width;
    u16 oldScaleX = *scaleX;
    f32 fov = 70.0f;

    // Like SBK2, latch the option when the first gameplay viewport is configured.
    if (startingRace && index == 0) {
        sVerticalTwoPlayerSplit = gPlayerCount == 2 && recomp_get_vertical_2p_split_screen_enabled();
    }
    if (index >= 2 || !sVerticalTwoPlayerSplit || gPlayerCount != 2 ||
        (!startingRace && gRaceCameras[index].initialized.value == 0)) {
        return fov;
    }

    // Transpose the authored inset rows, including the winner's transition back to a full-screen view.
    *x = 160 + (*y - 120) * 146 / 106;
    *y = 120 + (oldX - 160) * 106 / 146;
    *width = *height * 18 / 13;
    *height = oldWidth * 13 / 18;
    *scaleX = *scaleY * 4 / 3;
    *scaleY = oldScaleX * 3 / 4;
    // Match SBK2's 110-degree vertical-split camera and return to 70 degrees as the winner expands.
    fov += 30.0f * (*aspect - 4.0f / 3.0f);
    *aspect = (16.0f / 9.0f) / *aspect;
    return fov;
}

extern const f32 gDefaultViewportOverlayFarClip[1];
extern const f32 gCustomViewportOverlayFarClip[1];
extern const f32 gRaceViewportOverlayFarClip[1];
extern const f32 gMenuViewportFarClip[1];
extern const f32 gMenuViewportOverlayFarClip[4];

static s16 snapLowEdge(s16 value, s16 boundary) {
    if ((value > boundary) && ((value - boundary) <= VIEWPORT_EDGE_SNAP)) {
        return boundary;
    }
    return value;
}

static s16 snapHighEdge(s16 value, s16 boundary) {
    if ((value < boundary) && ((boundary - value) <= VIEWPORT_EDGE_SNAP)) {
        return boundary;
    }
    return value;
}

static void snapViewportBoundsToScreenEdges(ViewportState *viewport) {
    viewport->left = snapLowEdge(viewport->left, 0);
    viewport->left = snapLowEdge(viewport->left, SCREEN_WIDTH / 2);

    viewport->top = snapLowEdge(viewport->top, 0);
    viewport->top = snapLowEdge(viewport->top, SCREEN_HEIGHT / 2);

    viewport->right = snapHighEdge(viewport->right, SCREEN_WIDTH / 2);
    viewport->right = snapHighEdge(viewport->right, SCREEN_WIDTH);

    viewport->bottom = snapHighEdge(viewport->bottom, SCREEN_HEIGHT / 2);
    viewport->bottom = snapHighEdge(viewport->bottom, SCREEN_HEIGHT);

    viewport->viewport.vp.vtrans[0] = ((viewport->left + viewport->right) / 2) * 4;
    viewport->viewport.vp.vtrans[1] = ((viewport->top + viewport->bottom) / 2) * 4;
}

RECOMP_PATCH void configureViewport(
    s32 viewportIndex,
    s32 centerX,
    s32 centerY,
    u16 width,
    u16 height,
    u16 scaleX,
    u16 scaleY,
    f32 aspect
) {
    f32 fov = configureRaceSplitDirection(viewportIndex, &centerX, &centerY, &width, &height,
                                         &scaleX, &scaleY, &aspect);
    f32 trainingProgress = configureTrainingViewportTransition(viewportIndex);
    if (viewportIndex == 0) {
        sTrainingViewportTransitionProgress = trainingProgress;
    }
    if (trainingProgress > 0.0f) {
        // @recomp Remove the 16px inset gradually so each edge expands smoothly through training's zoom.
        width += gCurrentGameTask->callbackData0 * 2;
        height += gCurrentGameTask->callbackData0 * 2;
    }
    if (isPodiumViewport(viewportIndex)) {
        // Reveal the top and bottom of the pass-award scene without changing its
        // projection, camera, or the authored size and position of its text.
        width = SCREEN_WIDTH;
        height = SCREEN_HEIGHT;
    }
    gViewportStates[viewportIndex].active = 1;
    gViewportStates[viewportIndex].viewport.vp.vtrans[0] = centerX * 4;
    gViewportStates[viewportIndex].viewport.vp.vtrans[1] = centerY * 4;
    gViewportStates[viewportIndex].viewport.vp.vscale[0] = scaleX * 2;
    gViewportStates[viewportIndex].viewport.vp.vscale[1] = scaleY * 2;

    gViewportStates[viewportIndex].left = centerX - (width / 2);
    gViewportStates[viewportIndex].top = centerY - (height / 2);
    gViewportStates[viewportIndex].right = (width / 2) + centerX;
    gViewportStates[viewportIndex].bottom = (height / 2) + centerY;
    gViewportStates[viewportIndex].left = gViewportStates[viewportIndex].left;
    gViewportStates[viewportIndex].top = gViewportStates[viewportIndex].top;
    gViewportStates[viewportIndex].right = gViewportStates[viewportIndex].right;
    gViewportStates[viewportIndex].bottom = gViewportStates[viewportIndex].bottom;
    gViewportStates[viewportIndex].screenBoundsValid = 1;

    if (gViewportStates[viewportIndex].right < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].bottom < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left >= 0x140) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].top >= 0xF0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left < 0) {
        gViewportStates[viewportIndex].left = 0;
    }
    if (gViewportStates[viewportIndex].top < 0) {
        gViewportStates[viewportIndex].top = 0;
    }
    // @recomp Preserve the framebuffer edge instead of clamping one pixel inside it.
    if (gViewportStates[viewportIndex].right > 0x140) {
        gViewportStates[viewportIndex].right = 0x140;
    }
    if (gViewportStates[viewportIndex].bottom > 0xF0) {
        gViewportStates[viewportIndex].bottom = 0xF0;
    }

    // @recomp Expand near-edge bounds and recenter the rendered viewport to match them.
    if (trainingProgress < 0.0f) {
        snapViewportBoundsToScreenEdges(&gViewportStates[viewportIndex]);
    }
    
    guPerspective(
        &gViewportStates[viewportIndex].projectionMatrix,
        &gViewportStates[viewportIndex].perspectiveNorm,
        fov,
        aspect,
        10.0f,
        2800.0f,
        0.5f
    );
    guPerspective(
        &gViewportStates[viewportIndex].overlayProjectionMatrix,
        &gViewportStates[viewportIndex].overlayPerspectiveNorm,
        fov,
        aspect,
        10.0f,
        gDefaultViewportOverlayFarClip[0],
        0.5f
    );
}

RECOMP_PATCH void configureViewportWithFovAndFarClip(
    s32 viewportIndex,
    s32 centerX,
    s32 centerY,
    u16 width,
    u16 height,
    u16 scaleX,
    u16 scaleY,
    f32 aspect,
    s16 fovY,
    s32 farClip
) {
    s32 endingCredits = gCurrentGameTask != NULL &&
                        gCurrentGameTask->callbacks[0] == initEndingCreditsFlow;

    // @recomp Disable training's aspect transition when the custom-FOV path takes over viewport zero.
    if (viewportIndex == 0) {
        sTrainingViewportTransitionProgress = -1.0f;
    }
    gViewportStates[viewportIndex].active = 1;
    gViewportStates[viewportIndex].viewport.vp.vtrans[0] = centerX * 4;
    gViewportStates[viewportIndex].viewport.vp.vtrans[1] = centerY * 4;
    gViewportStates[viewportIndex].viewport.vp.vscale[0] = scaleX * 2;
    gViewportStates[viewportIndex].viewport.vp.vscale[1] = scaleY * 2;

    gViewportStates[viewportIndex].left = centerX - (width / 2);
    gViewportStates[viewportIndex].top = centerY - (height / 2);
    gViewportStates[viewportIndex].right = (width / 2) + centerX;
    gViewportStates[viewportIndex].bottom = (height / 2) + centerY;
    gViewportStates[viewportIndex].left = gViewportStates[viewportIndex].left;
    gViewportStates[viewportIndex].top = gViewportStates[viewportIndex].top;
    gViewportStates[viewportIndex].right = gViewportStates[viewportIndex].right;
    gViewportStates[viewportIndex].bottom = gViewportStates[viewportIndex].bottom;
    gViewportStates[viewportIndex].screenBoundsValid = 1;

    if (gViewportStates[viewportIndex].right < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].bottom < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left >= 0x140) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].top >= 0xF0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left < 0) {
        gViewportStates[viewportIndex].left = 0;
    }
    if (gViewportStates[viewportIndex].top < 0) {
        gViewportStates[viewportIndex].top = 0;
    }
    // @recomp Preserve the framebuffer edge instead of clamping one pixel inside it.
    if (gViewportStates[viewportIndex].right > 0x140) {
        gViewportStates[viewportIndex].right = 0x140;
    }
    if (gViewportStates[viewportIndex].bottom > 0xF0) {
        gViewportStates[viewportIndex].bottom = 0xF0;
    }

    // @recomp The ending credits use their 16-pixel inset to clip characters as they enter and leave the scene.
    // Preserve that authored pillarbox instead of exposing the off-screen animation in widescreen.
    if (!endingCredits) {
        snapViewportBoundsToScreenEdges(&gViewportStates[viewportIndex]);
    }
   
    guPerspective(
        &gViewportStates[viewportIndex].projectionMatrix,
        &gViewportStates[viewportIndex].perspectiveNorm,
        (f32)fovY,
        aspect,
        10.0f,
        (f32)farClip,
        0.5f
    );
    guPerspective(
        &gViewportStates[viewportIndex].overlayProjectionMatrix,
        &gViewportStates[viewportIndex].overlayPerspectiveNorm,
        (f32)fovY,
        aspect,
        10.0f,
        gCustomViewportOverlayFarClip[0],
        0.5f
    );
}

RECOMP_PATCH void configureRaceViewport(
    s32 viewportIndex,
    s32 centerX,
    s32 centerY,
    u16 width,
    u16 height,
    u16 scaleX,
    u16 scaleY,
    f32 aspect
) {
    f32 fov = configureRaceSplitDirection(viewportIndex, &centerX, &centerY, &width, &height,
                                         &scaleX, &scaleY, &aspect);
    // @recomp Disable training's aspect transition when the race path takes over viewport zero.
    if (viewportIndex == 0) {
        sTrainingViewportTransitionProgress = -1.0f;
    }
    gViewportStates[viewportIndex].active = 1;
    gViewportStates[viewportIndex].viewport.vp.vtrans[0] = centerX * 4;
    gViewportStates[viewportIndex].viewport.vp.vtrans[1] = centerY * 4;
    gViewportStates[viewportIndex].viewport.vp.vscale[0] = scaleX * 2;
    gViewportStates[viewportIndex].viewport.vp.vscale[1] = scaleY * 2;

    gViewportStates[viewportIndex].left = centerX - (width / 2);
    gViewportStates[viewportIndex].top = centerY - (height / 2);
    gViewportStates[viewportIndex].right = (width / 2) + centerX;
    gViewportStates[viewportIndex].bottom = (height / 2) + centerY;
    gViewportStates[viewportIndex].left = gViewportStates[viewportIndex].left;
    gViewportStates[viewportIndex].top = gViewportStates[viewportIndex].top;
    gViewportStates[viewportIndex].right = gViewportStates[viewportIndex].right;
    gViewportStates[viewportIndex].bottom = gViewportStates[viewportIndex].bottom;
    gViewportStates[viewportIndex].screenBoundsValid = 1;

    if (gViewportStates[viewportIndex].right < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].bottom < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left >= 0x140) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].top >= 0xF0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left < 0) {
        gViewportStates[viewportIndex].left = 0;
    }
    if (gViewportStates[viewportIndex].top < 0) {
        gViewportStates[viewportIndex].top = 0;
    }
    // @recomp Preserve the framebuffer edge instead of clamping one pixel inside it.
    if (gViewportStates[viewportIndex].right > 0x140) {
        gViewportStates[viewportIndex].right = 0x140;
    }
    if (gViewportStates[viewportIndex].bottom > 0xF0) {
        gViewportStates[viewportIndex].bottom = 0xF0;
    }

    // @recomp Expand near-edge bounds and recenter the rendered viewport to match them.
    snapViewportBoundsToScreenEdges(&gViewportStates[viewportIndex]);
    
    guPerspective(
        &gViewportStates[viewportIndex].projectionMatrix,
        &gViewportStates[viewportIndex].perspectiveNorm,
        fov,
        aspect,
        10.0f,
        1000.0f,
        0.5f
    );
    guPerspective(
        &gViewportStates[viewportIndex].overlayProjectionMatrix,
        &gViewportStates[viewportIndex].overlayPerspectiveNorm,
        fov,
        aspect,
        10.0f,
        gRaceViewportOverlayFarClip[0],
        0.5f
    );
}

RECOMP_PATCH void configureMenuViewport(
    s32 viewportIndex,
    s32 centerX,
    s32 centerY,
    u16 width,
    u16 height,
    u16 scaleX,
    u16 scaleY,
    f32 aspect
) {
    // @recomp Disable training's aspect transition when the menu path takes over viewport zero.
    if (viewportIndex == 0) {
        sTrainingViewportTransitionProgress = -1.0f;
    }
    gViewportStates[viewportIndex].active = 1;
    gViewportStates[viewportIndex].viewport.vp.vtrans[0] = centerX * 4;
    gViewportStates[viewportIndex].viewport.vp.vtrans[1] = centerY * 4;
    gViewportStates[viewportIndex].viewport.vp.vscale[0] = scaleX * 2;
    gViewportStates[viewportIndex].viewport.vp.vscale[1] = scaleY * 2;

    gViewportStates[viewportIndex].left = centerX - (width / 2);
    gViewportStates[viewportIndex].top = centerY - (height / 2);
    gViewportStates[viewportIndex].right = (width / 2) + centerX;
    gViewportStates[viewportIndex].bottom = (height / 2) + centerY;
    gViewportStates[viewportIndex].left = gViewportStates[viewportIndex].left;
    gViewportStates[viewportIndex].top = gViewportStates[viewportIndex].top;
    gViewportStates[viewportIndex].right = gViewportStates[viewportIndex].right;
    gViewportStates[viewportIndex].bottom = gViewportStates[viewportIndex].bottom;
    gViewportStates[viewportIndex].screenBoundsValid = 1;

    if (gViewportStates[viewportIndex].right < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].bottom < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left >= 0x140) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].top >= 0xF0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left < 0) {
        gViewportStates[viewportIndex].left = 0;
    }
    if (gViewportStates[viewportIndex].top < 0) {
        gViewportStates[viewportIndex].top = 0;
    }
    // @recomp Preserve the framebuffer edge instead of clamping one pixel inside it.
    if (gViewportStates[viewportIndex].right > 0x140) {
        gViewportStates[viewportIndex].right = 0x140;
    }
    if (gViewportStates[viewportIndex].bottom > 0xF0) {
        gViewportStates[viewportIndex].bottom = 0xF0;
    }

    // @recomp Expand near-edge bounds and recenter the rendered viewport to match them.
    snapViewportBoundsToScreenEdges(&gViewportStates[viewportIndex]);
    
    guPerspective(
        &gViewportStates[viewportIndex].projectionMatrix,
        &gViewportStates[viewportIndex].perspectiveNorm,
        70.0f,
        aspect,
        10.0f,
        gMenuViewportFarClip[0],
        0.5f
    );
    guPerspective(
        &gViewportStates[viewportIndex].overlayProjectionMatrix,
        &gViewportStates[viewportIndex].overlayPerspectiveNorm,
        70.0f,
        aspect,
        10.0f,
        gMenuViewportOverlayFarClip[0],
        0.5f
    );
}
