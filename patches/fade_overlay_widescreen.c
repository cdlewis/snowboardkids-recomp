#include "patches.h"

#include "transform_ids.h"
#include "camera_interpolation.h"
#include "race_split_screen.h"
#include "podium_scene.h"

#include "game/engine/render_callback.h"
#include "game/engine/viewport_manager.h"
#include "game/race/camera/race_camera.h"
#include "game/math/fixed_point_math.h"
#include "game/race/ui/race_hud.h"

extern s16 gUiBlinkTimer;
extern s16 gMenuViewportWidth;
extern s16 gMenuViewportHeight;
extern s16 gMenuViewportCenterX;
extern s16 gMenuViewportCenterY;
extern u8 gCurrentViewportIndex;
extern u8 gRenderMatricesDirty;
extern s16 gMenuFadeAlpha;
extern u8 gMenuFadeOverlayActive;
extern Gfx *gRegionAllocPtr;
extern Mtx *gViewportMatrix;

extern RenderCallbackNode *gMenuOverlayRenderCallbackList;
extern RenderCallbackNode *gMenuRenderCallbackList;
extern RenderCallbackNode *gMenuForegroundRenderCallbackList;
extern RenderCallbackNode *gRaceForegroundRenderCallbackList;
extern RenderCallbackNode *gRaceOverlayRenderCallbackList;
extern RenderCallbackNode *gBackdropRenderCallbackList;
extern RenderCallbackNode *gModelRenderCallbackList;
extern RenderCallbackNode *gEffectRenderCallbackList;
extern RenderCallbackNode *D_80124848;

extern Gfx gMenuRenderModeResetDl[];
extern FrameRenderTask gFrameRenderTasks[];

extern void runRenderCallbacks(RenderCallbackNode **list);
extern void appendFadeOverlayDisplayList(void);
extern void initMenuAsciiFontTexture(void);

#define VIEWPORT_COUNT 4

static s32 raceViewportUsesColumns(s32 index) {
    ViewportState *viewport = &gViewportStates[index];
    return gRaceCameras[index].initialized.value != 0 && viewport->screenBoundsValid != 0 &&
           viewport->right - viewport->left <= FRAMEBUFFER_WIDTH / 2;
}

static void setViewportScissorAlignment(s32 index) {
    ViewportState *viewport = &gViewportStates[index];
    if (raceViewportUsesColumns(index)) {
        if (viewport->left < FRAMEBUFFER_WIDTH / 2) {
            gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_LEFT, G_EX_ORIGIN_CENTER,
                              0, 0, -FRAMEBUFFER_WIDTH / 2, 0, 0, 0,
                              FRAMEBUFFER_WIDTH / 2, FRAMEBUFFER_HEIGHT);
        } else {
            gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_CENTER, G_EX_ORIGIN_RIGHT,
                              -FRAMEBUFFER_WIDTH / 2, 0, -FRAMEBUFFER_WIDTH, 0,
                              FRAMEBUFFER_WIDTH / 2, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        }
    } else {
        gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE,
                          0, 0, 0, 0, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    }
}

static void emitRaceViewport(void) {
    Vp *viewport = &gCurrentFrameRenderData->viewport.viewports[gCurrentViewportIndex];
    if (raceViewportUsesColumns(gCurrentViewportIndex) ||
        (gCurrentViewportIndex == 2 && isPodiumViewport(gCurrentViewportIndex))) {
        gEXViewport(gRegionAllocPtr++, G_EX_ORIGIN_CENTER, viewport);
    } else {
        gSPViewport(gRegionAllocPtr++, viewport);
    }
}

static void drawRaceViewportDividers(void) {
    RenderCallbackNode *hud = gRaceOverlayRenderCallbackList;
    s32 viewportCount = 0;
    s32 i;

    // Use this frame's HUD work, not the HUD mode retained while moving between screens.
    // Original game code queues the US dump's address even though the HUD implementation is replaced.
    const u32 originalTwoPlayerHudAddress = 0x800799DC;
    const u32 originalMultiplayerHudAddress = 0x80079F04;
    while (hud != NULL && (u32)hud->callback != originalTwoPlayerHudAddress &&
           (u32)hud->callback != originalMultiplayerHudAddress &&
           hud->callback != drawTwoPlayerRaceHud && hud->callback != drawMultiplayerRaceHud) {
        hud = hud->next;
    }
    if (hud == NULL) {
        return;
    }

    for (i = 0; i < VIEWPORT_COUNT; i++) {
        if (gRaceCameras[i].initialized.value != 0 && gViewportStates[i].screenBoundsValid != 0) {
            viewportCount++;
        }
    }
    if (viewportCount < 2) {
        return;
    }

    gEXPushScissor(gRegionAllocPtr++);
    gEXPushOtherMode(gRegionAllocPtr++);
    gEXPushFillColor(gRegionAllocPtr++);
    gEXPushCombineMode(gRegionAllocPtr++);
    gEXPushPrimColor(gRegionAllocPtr++);
    gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_LEFT, G_EX_ORIGIN_RIGHT,
                      0, 0, -FRAMEBUFFER_WIDTH, 0, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    gDPSetScissor(gRegionAllocPtr++, G_SC_NON_INTERLACE, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_AUTO);
    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
    gDPPipeSync(gRegionAllocPtr++);
    gDPSetCycleType(gRegionAllocPtr++, G_CYC_FILL);
    gDPSetRenderMode(gRegionAllocPtr++, G_RM_NOOP, G_RM_NOOP2);
    gDPSetFillColor(gRegionAllocPtr++, 0x00010001);

    // Fill-cycle endpoints are inclusive, restoring the original two-native-pixel gaps.
    if (viewportCount == 2 && !raceUsesVerticalTwoPlayerSplit()) {
        // Use output-edge origins rather than the independently configured HUD width.
        gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_ADJUST);
        gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_LEFT, G_EX_ORIGIN_RIGHT,
                       0, 0, -(FRAMEBUFFER_WIDTH - 1) * 4, 0);
    }
    if (viewportCount >= 3 || !raceUsesVerticalTwoPlayerSplit()) {
        gDPFillRectangle(gRegionAllocPtr++, 0, FRAMEBUFFER_HEIGHT / 2 - 1,
                         FRAMEBUFFER_WIDTH - 1, FRAMEBUFFER_HEIGHT / 2);
    }

    if (viewportCount >= 3 || raceUsesVerticalTwoPlayerSplit()) {
        // Draw as an opaque rectangle, not a fill-cycle clear: clear endpoints round
        // outwards (and explicit origins quantize them again), making the vertical arm
        // wider than the horizontal gap at non-integer output scales.
        gDPPipeSync(gRegionAllocPtr++);
        gDPSetCycleType(gRegionAllocPtr++, G_CYC_1CYCLE);
        gDPSetRenderMode(gRegionAllocPtr++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        gDPSetCombineMode(gRegionAllocPtr++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0, 0, 0, 255);
        gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_ADJUST);
        // Use the progress meter's explicit centre so output-scale rounding stays aligned.
        gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_CENTER, G_EX_ORIGIN_CENTER,
                       -FRAMEBUFFER_WIDTH * 2 + 4, 0, -FRAMEBUFFER_WIDTH * 2 + 4, 0);
        // Unlike fill-cycle endpoints, one-cycle rectangle endpoints are exclusive.
        gDPFillRectangle(gRegionAllocPtr++, FRAMEBUFFER_WIDTH / 2 - 1, 0,
                         FRAMEBUFFER_WIDTH / 2 + 1, FRAMEBUFFER_HEIGHT);
    }

    gDPPipeSync(gRegionAllocPtr++);
    gEXPopPrimColor(gRegionAllocPtr++);
    gEXPopCombineMode(gRegionAllocPtr++);
    gEXPopFillColor(gRegionAllocPtr++);
    gEXPopOtherMode(gRegionAllocPtr++);
    gEXPopScissor(gRegionAllocPtr++);
    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
}

typedef struct {
    Mat3x3 rotation;
    u32 frame;
    u8 valid;
    u8 skipInterpolation;
} ViewportCameraHistory;


static ViewportCameraHistory sViewportCameraHistory[2][VIEWPORT_COUNT];
static u32 sViewportRenderFrame;

void invalidateViewportCameraInterpolation(u32 viewportIndex) {
    sViewportCameraHistory[0][viewportIndex].valid = 0;
    sViewportCameraHistory[1][viewportIndex].valid = 0;
}

static s32 viewportCameraRotationCut(u32 base) {
    ViewportCameraHistory *history =
        &sViewportCameraHistory[base == PROJECTION_VIEWPORT_MAIN_ID_BASE][gCurrentViewportIndex];
    s16 *rotation = gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation;
    s64 dotSum = 0;
    s64 traceThreshold;
    s32 cut;
    s32 i;

    traceThreshold = (s64)(FIXED_MATRIX_ONE + 2 * fixedCosine(20 * 4096 / 360)) * FIXED_MATRIX_ONE;
    for (i = 0; i < 9; i++) {
        dotSum += (s32)history->rotation[i] * rotation[i];
        history->rotation[i] = rotation[i];
    }
    cut = !history->valid || history->frame + 1 != sViewportRenderFrame || dotSum < traceThreshold;
    history->skipInterpolation = cut;
    history->valid = 1;
    history->frame = sViewportRenderFrame;
    return cut;
}

s32 viewportCameraSkipsInterpolation(void) {
    return sViewportCameraHistory[1][gCurrentViewportIndex].skipInterpolation;
}

static void pushViewportProjectionMatrixGroup(u32 base) {
    u32 id = base | gCurrentViewportIndex;
    u32 component = viewportCameraRotationCut(base) ? G_EX_COMPONENT_SKIP : G_EX_COMPONENT_INTERPOLATE;
    u32 aspect = raceViewportUsesColumns(gCurrentViewportIndex) ? G_EX_ASPECT_ADJUST : G_EX_ASPECT_AUTO;

    if (gRaceCameras[gCurrentViewportIndex].initialized.value != 0) {
        id |= PROJECTION_VIEWPORT_RACE_CONTEXT_BIT;
    }

    // @recomp Snap the entire camera projection on a cut, including translation, so no intermediate shot is drawn.
    gEXMatrixGroup(
        gRegionAllocPtr++, id, G_EX_INTERPOLATE_SIMPLE, G_EX_PUSH, G_MTX_PROJECTION,
        component, component, component,
        component, component, G_EX_COMPONENT_SKIP,
        component, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE, aspect,
        G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO
    );
}

RECOMP_PATCH void appendViewportDisplayLists(u8 frameIndex) {
    RenderCallbackNode **queue;
    s32 hasModelCallbacks;
    u32 upperMask;
    s16 left;
    s16 top;
    s32 i;
    s32 splitFrame = 0;
    s32 raceFrame = 0;
    f32 aspectScale = recomp_get_target_aspect_ratio(4.0f / 3.0f) / (4.0f / 3.0f);

    sViewportRenderFrame++;
    for (i = 0; i < VIEWPORT_COUNT; i++) {
        splitFrame |= raceViewportUsesColumns(i);
        raceFrame |= gRaceCameras[i].initialized.value != 0 && gViewportStates[i].screenBoundsValid != 0;
    }

    gUiBlinkTimer++;
    gMenuViewportWidth = 288;
    gMenuViewportHeight = 208;
    gMenuViewportCenterX = 160;
    gMenuViewportCenterY = 120;

    gDPPipeSync(gRegionAllocPtr++);
    gEXSetViewportAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, 0, 0);
    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
    gEXSetRectAspect(gRegionAllocPtr++, splitFrame ? G_EX_ASPECT_ADJUST : G_EX_ASPECT_AUTO);
    gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE,
                      0, 0, 0, 0, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    gDPSetScissor(
        gRegionAllocPtr++, G_SC_NON_INTERLACE, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT
    );
    gSPViewport(gRegionAllocPtr++, gFullscreenOverlayViewport);

    if (gMenuOverlayRenderCallbackList != NULL) {
        gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
        runRenderCallbacks(&gMenuOverlayRenderCallbackList);
    }

    appendFadeOverlayDisplayList();
    gRenderMatricesDirty = 1;

    upperMask = 0xFFFF0000;
    for (gCurrentViewportIndex = 0; gCurrentViewportIndex < VIEWPORT_COUNT; gCurrentViewportIndex++) {
        if (gViewportStates[gCurrentViewportIndex].screenBoundsValid != 0) {
            gCurrentFrameRenderData->viewport.viewports[gCurrentViewportIndex] =
                gViewportStates[gCurrentViewportIndex].viewport;
            if (raceViewportUsesColumns(gCurrentViewportIndex)) {
                Vp *viewport = &gCurrentFrameRenderData->viewport.viewports[gCurrentViewportIndex];
                f32 center = (viewport->vp.vtrans[0] - FRAMEBUFFER_WIDTH * 2) * aspectScale;
                // SBK2 widens a frame-local viewport and submits its centre relative to the output centre.
                viewport->vp.vscale[0] = (s16)(viewport->vp.vscale[0] * aspectScale + 0.5f);
                viewport->vp.vtrans[0] = (s16)(center + (center < 0.0f ? -0.5f : 0.5f));
            } else if (gCurrentViewportIndex == 2 && isPodiumViewport(gCurrentViewportIndex)) {
                // The rotating congratulations banner and its afterimages use
                // viewport 2 before becoming a centred 2D sprite. Keep this
                // overlay viewport at native aspect so the handoff cannot change
                // the banner's width. Viewports 0 and 1 still widen the scene.
                gCurrentFrameRenderData->viewport.viewports[gCurrentViewportIndex].vp.vtrans[0] -=
                    FRAMEBUFFER_WIDTH * 2;
            }
            gCurrentFrameRenderData->viewport.projections[gCurrentViewportIndex] =
                gViewportStates[gCurrentViewportIndex].projectionMatrix;
            gCurrentFrameRenderData->viewport.overlayProjections[gCurrentViewportIndex] =
                gViewportStates[gCurrentViewportIndex].overlayProjectionMatrix;

            left = gViewportStates[gCurrentViewportIndex].left;
            top = gViewportStates[gCurrentViewportIndex].top;
            gMenuViewportWidth = gViewportStates[gCurrentViewportIndex].right - left;
            gMenuViewportHeight = gViewportStates[gCurrentViewportIndex].bottom - top;
            gMenuViewportCenterX = left + (gMenuViewportWidth / 2);
            gMenuViewportCenterY = top + (gMenuViewportHeight / 2);

            gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex] =
                gRaceCameras[gCurrentViewportIndex].packedTransform;
            gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex].m[1][2] = 0;
            gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex].m[1][3] = 1;
            gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex].m[3][2] = 0;
            gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex].m[3][3] = 0;
            gViewportMatrix = &gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex];

            gDPPipeSync(gRegionAllocPtr++);
            setViewportScissorAlignment(gCurrentViewportIndex);
            gDPSetScissor(
                gRegionAllocPtr++,
                G_SC_NON_INTERLACE,
                gViewportStates[gCurrentViewportIndex].left,
                gViewportStates[gCurrentViewportIndex].top,
                gViewportStates[gCurrentViewportIndex].right,
                gViewportStates[gCurrentViewportIndex].bottom
            );
            hasModelCallbacks = 0;

            if (gViewportStates[gCurrentViewportIndex].clearFramebuffer != 0) {
                gDPPipeSync(gRegionAllocPtr++);
                gDPSetCycleType(gRegionAllocPtr++, G_CYC_FILL);
                gDPSetRenderMode(gRegionAllocPtr++, G_RM_NOOP, G_RM_NOOP2);
                gDPSetColorImage(gRegionAllocPtr++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gDepthBuffer);
                gDPSetFillColor(gRegionAllocPtr++, 0xFFFCFFFC);
                gDPFillRectangle(
                    gRegionAllocPtr++,
                    gViewportStates[gCurrentViewportIndex].left,
                    gViewportStates[gCurrentViewportIndex].top,
                    gViewportStates[gCurrentViewportIndex].right - 1,
                    gViewportStates[gCurrentViewportIndex].bottom - 1
                );
                gDPPipeSync(gRegionAllocPtr++);
                gDPSetColorImage(
                    gRegionAllocPtr++,
                    G_IM_FMT_RGBA,
                    G_IM_SIZ_16b,
                    320,
                    gFrameRenderTasks[frameIndex].framebuffer
                );
            }

            if (gViewportStates[gCurrentViewportIndex].overlayActive != 0) {
                gDPPipeSync(gRegionAllocPtr++);
                gDPSetCycleType(gRegionAllocPtr++, G_CYC_FILL);
                gDPSetRenderMode(gRegionAllocPtr++, G_RM_NOOP, G_RM_NOOP2);
                gDPSetColorImage(
                    gRegionAllocPtr++,
                    G_IM_FMT_RGBA,
                    G_IM_SIZ_16b,
                    320,
                    gFrameRenderTasks[frameIndex].framebuffer
                );
                gDPSetFillColor(
                    gRegionAllocPtr++,
                    (GPACK_RGBA5551(
                         (u8)gViewportStates[gCurrentViewportIndex].overlayR,
                         (u8)gViewportStates[gCurrentViewportIndex].overlayG,
                         (u8)gViewportStates[gCurrentViewportIndex].overlayB,
                         1
                     )
                     << 16) |
                        GPACK_RGBA5551(
                            (u8)gViewportStates[gCurrentViewportIndex].overlayR,
                            (u8)gViewportStates[gCurrentViewportIndex].overlayG,
                            (u8)gViewportStates[gCurrentViewportIndex].overlayB,
                            1
                        )
                );
                gDPFillRectangle(
                    gRegionAllocPtr++,
                    gViewportStates[gCurrentViewportIndex].left,
                    gViewportStates[gCurrentViewportIndex].top,
                    gViewportStates[gCurrentViewportIndex].right - 1,
                    gViewportStates[gCurrentViewportIndex].bottom - 1
                );
            }

            if (D_80124848 != NULL) {
                gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
                runRenderCallbacks(&D_80124848);
            }

            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[0][0] =
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[0] << 4) & upperMask) |
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[1] >> 12) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[0][1] =
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[2] << 4) & upperMask;
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[0][2] =
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[3] << 4) & upperMask) |
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[4] >> 12) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[0][3] =
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[5] << 4) & upperMask;
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[1][0] =
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[6] << 4) & upperMask) |
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[7] >> 12) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[1][1] =
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[8] << 4) & upperMask;
            gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex].m[1][2] =
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.translation.x & upperMask) |
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.translation.y >> 16) & 0xFFFF);
            gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex].m[1][3] =
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.translation.z & upperMask) | 1;

            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[2][0] =
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[0] << 20) & upperMask) |
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[1] << 4) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[2][1] =
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[2] << 20) & upperMask;
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[2][2] =
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[3] << 20) & upperMask) |
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[4] << 4) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[2][3] =
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[5] << 20) & upperMask;
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[3][0] =
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[6] << 20) & upperMask) |
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[7] << 4) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[3][1] =
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.rotation[8] << 20) & upperMask;
            gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex].m[3][2] =
                ((gRaceCameras[gCurrentViewportIndex].cameraTransform.translation.x << 16) & upperMask) |
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.translation.y & 0xFFFF);
            gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex].m[3][3] =
                (gRaceCameras[gCurrentViewportIndex].cameraTransform.translation.z << 16) & upperMask;

            if (gBackdropRenderCallbackList != NULL) {
                pushViewportProjectionMatrixGroup(PROJECTION_VIEWPORT_BACKDROP_ID_BASE);
                gSPPerspNormalize(
                    gRegionAllocPtr++,
                    gViewportStates[gCurrentViewportIndex].overlayPerspectiveNorm
                );
                gSPMatrix(
                    gRegionAllocPtr++,
                    &gCurrentFrameRenderData->viewport.overlayProjections[gCurrentViewportIndex],
                    G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION
                );
                emitRaceViewport();
                gSPMatrix(
                    gRegionAllocPtr++,
                    &gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex],
                    G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION
                );
                gSPMatrix(
                    gRegionAllocPtr++,
                    &gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex],
                    G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION
                );
                gSPDisplayList(gRegionAllocPtr++, gBackdropRenderSetupDisplayList);
                runRenderCallbacks(&gBackdropRenderCallbackList);
                gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_PROJECTION);
            }

            for (i = 0; i < MODEL_RENDER_CALLBACK_QUEUE_COUNT; i++) {
                if (gModelRenderCallbackQueues[i].head != NULL) {
                    hasModelCallbacks = 1;
                }
            }

            if (hasModelCallbacks != 0) {
                pushViewportProjectionMatrixGroup(PROJECTION_VIEWPORT_MAIN_ID_BASE);
                gSPPerspNormalize(gRegionAllocPtr++, gViewportStates[gCurrentViewportIndex].perspectiveNorm);
                gSPMatrix(
                    gRegionAllocPtr++,
                    &gCurrentFrameRenderData->viewport.projections[gCurrentViewportIndex],
                    G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION
                );
                emitRaceViewport();
                gSPMatrix(
                    gRegionAllocPtr++,
                    &gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex],
                    G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION
                );
                gSPMatrix(
                    gRegionAllocPtr++,
                    &gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex],
                    G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION
                );
                gSPDisplayList(gRegionAllocPtr++, gModelRenderSetupDisplayList);

                for (i = 0; i < MODEL_RENDER_CALLBACK_QUEUE_COUNT; i++) {
                    if (gModelRenderCallbackQueues[i].head != NULL) {
                        queue = &gModelRenderCallbackQueues[i].head;
                        if (queue == &gEffectRenderCallbackList) {
                            gSPMatrix(
                                gRegionAllocPtr++,
                                &gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex],
                                G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW
                            );
                        }
                        runRenderCallbacks(queue);
                    }
                }
                gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_PROJECTION);
            }

            if ((gRaceForegroundRenderCallbackList != NULL) || (gRaceOverlayRenderCallbackList != NULL)) {
                if (raceViewportUsesColumns(gCurrentViewportIndex)) {
                    // Keep the authored HUD group centred in its physical quadrant without stretching sprites.
                    f32 shift = (gMenuViewportCenterX - FRAMEBUFFER_WIDTH / 2) * (aspectScale - 1.0f) * 4.0f;
                    s32 offset = (s32)(shift + (shift < 0.0f ? -0.5f : 0.5f));
                    gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_ADJUST);
                    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE,
                                   offset, 0, offset, 0);
                }
                gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
                if (gRaceOverlayRenderCallbackList != NULL) {
                    runRenderCallbacks(&gRaceOverlayRenderCallbackList);
                }
                if (gRaceForegroundRenderCallbackList != NULL) {
                    initMenuAsciiFontTexture();
                    runRenderCallbacks(&gRaceForegroundRenderCallbackList);
                }
            }

            if (gViewportStates[gCurrentViewportIndex].overlayAlpha != 0) {
                gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_AUTO);
                gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
                gSPDisplayList(gRegionAllocPtr++, gTranslucentOverlaySetupDisplayList);
                gDPSetPrimColor(
                    gRegionAllocPtr++,
                    0,
                    0,
                    0,
                    0,
                    0,
                    gViewportStates[gCurrentViewportIndex].overlayAlpha
                );
                // @recomp Use the viewport bounds so full-screen fades reach the widened output edges.
                gSPTextureRectangle(
                    gRegionAllocPtr++,
                    gViewportStates[gCurrentViewportIndex].left << 2,
                    gViewportStates[gCurrentViewportIndex].top << 2,
                    gViewportStates[gCurrentViewportIndex].right << 2,
                    gViewportStates[gCurrentViewportIndex].bottom << 2,
                    G_TX_RENDERTILE,
                    0,
                    0,
                    1 << 10,
                    1 << 10
                );
            }

            gRenderMatricesDirty = 0;
            gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
        }
    }

    gMenuViewportWidth = 288;
    gMenuViewportHeight = 208;
    gMenuViewportCenterX = 160;
    gMenuViewportCenterY = 120;

    // Draw beneath the shared HUD so the meter, player markers, and countdown remain uninterrupted.
    drawRaceViewportDividers();
    gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_LEFT, G_EX_ORIGIN_RIGHT,
                      0, 0, -FRAMEBUFFER_WIDTH, 0, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    gEXSetRectAspect(gRegionAllocPtr++, splitFrame ? G_EX_ASPECT_ADJUST : G_EX_ASPECT_AUTO);

    if ((gMenuForegroundRenderCallbackList != NULL) || (gMenuRenderCallbackList != NULL)) {
        if (!raceFrame) {
            // @recomp Match the sprite helpers' horizontal menu bounds. Hidden paint-menu
            // panels can otherwise leave a border fragment in the 16-pixel side margin.
            gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE,
                              0, 0, 0, 0, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        }
        gDPSetScissor(gRegionAllocPtr++, G_SC_NON_INTERLACE,
                      raceFrame ? 0 : gMenuViewportCenterX - gMenuViewportWidth / 2, 0,
                      raceFrame ? FRAMEBUFFER_WIDTH : gMenuViewportCenterX + gMenuViewportWidth / 2,
                      FRAMEBUFFER_HEIGHT);
        gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
        if (gMenuRenderCallbackList != NULL) {
            runRenderCallbacks(&gMenuRenderCallbackList);
        }
        if (gMenuForegroundRenderCallbackList != NULL) {
            initMenuAsciiFontTexture();
            runRenderCallbacks(&gMenuForegroundRenderCallbackList);
        }
        if (!raceFrame) {
            // @recomp Restore wide alignment only after the menu pass changed it.
            // The following scissor then lets fades cover the full output.
            gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_LEFT, G_EX_ORIGIN_RIGHT,
                              0, 0, -FRAMEBUFFER_WIDTH, 0, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        }
    }

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetScissor(gRegionAllocPtr++, G_SC_NON_INTERLACE, 0, 0, 320, 240);

    if (gMenuFadeAlpha != 0) {
        gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_AUTO);
        gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
        gSPDisplayList(gRegionAllocPtr++, gTranslucentOverlaySetupDisplayList);
        if (gMenuFadeOverlayActive != 0) {
            gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 255, 255, 255, gMenuFadeAlpha);
        } else {
            gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0, 0, 0, gMenuFadeAlpha);
        }
        // @recomp Use the framebuffer bounds so the global fade reaches the widened output edges.
        gSPTextureRectangle(
            gRegionAllocPtr++,
            0,
            0,
            FRAMEBUFFER_WIDTH << 2,
            FRAMEBUFFER_HEIGHT << 2,
            G_TX_RENDERTILE,
            0,
            0,
            1 << 10,
            1 << 10
        );
    }

    // Frame setup can clear buffers before this function runs again; do not leak rectangle state into it.
    gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_AUTO);
    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
    gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE,
                      0, 0, 0, 0, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
}

#undef VIEWPORT_COUNT
