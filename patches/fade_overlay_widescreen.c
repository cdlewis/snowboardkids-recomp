#include "patches.h"

#include "game/engine/render_callback.h"
#include "game/engine/viewport_manager.h"
#include "game/race/camera/race_camera.h"

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

extern Vp D_800DEF18[];
extern Gfx D_800DEF28[];
extern Gfx D_800DEF90[];
extern Gfx D_800DF098[];
extern Gfx gMenuRenderModeResetDl[];
extern RaceCamera D_801121E0[RACE_CAMERA_COUNT];
extern FrameRenderTask gFrameRenderTasks[];

extern void runRenderCallbacks(RenderCallbackNode **list);
extern void appendFadeOverlayDisplayList(void);
extern void initMenuAsciiFontTexture(void);

#define runtimeModelRenderCallbackLists (*(RenderCallbackNode * (*)[24]) & gModelRenderCallbackList)
#define VIEWPORT_COUNT 4

RECOMP_PATCH void appendViewportDisplayLists(u8 frameIndex) {
    RenderCallbackNode **queue;
    s32 hasModelCallbacks;
    u32 upperMask;
    s16 left;
    s16 top;
    s32 i;

    gUiBlinkTimer++;
    gMenuViewportWidth = 288;
    gMenuViewportHeight = 208;
    gMenuViewportCenterX = 160;
    gMenuViewportCenterY = 120;

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetScissor(
        gRegionAllocPtr++, G_SC_NON_INTERLACE, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT
    );
    gSPViewport(gRegionAllocPtr++, D_800DEF18);

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
                D_801121E0[gCurrentViewportIndex].packedTransform;
            gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex].m[1][2] = 0;
            gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex].m[1][3] = 1;
            gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex].m[3][2] = 0;
            gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex].m[3][3] = 0;
            gViewportMatrix = &gCurrentFrameRenderData->viewport.viewportMatrices[gCurrentViewportIndex];

            gDPPipeSync(gRegionAllocPtr++);
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
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[0] << 4) & upperMask) |
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[1] >> 12) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[0][1] =
                (D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[2] << 4) & upperMask;
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[0][2] =
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[3] << 4) & upperMask) |
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[4] >> 12) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[0][3] =
                (D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[5] << 4) & upperMask;
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[1][0] =
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[6] << 4) & upperMask) |
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[7] >> 12) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[1][1] =
                (D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[8] << 4) & upperMask;
            gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex].m[1][2] =
                (D_801121E0[gCurrentViewportIndex].cameraTransform.translation.x & upperMask) |
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.translation.y >> 16) & 0xFFFF);
            gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex].m[1][3] =
                (D_801121E0[gCurrentViewportIndex].cameraTransform.translation.z & upperMask) | 1;

            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[2][0] =
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[0] << 20) & upperMask) |
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[1] << 4) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[2][1] =
                (D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[2] << 20) & upperMask;
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[2][2] =
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[3] << 20) & upperMask) |
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[4] << 4) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[2][3] =
                (D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[5] << 20) & upperMask;
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[3][0] =
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[6] << 20) & upperMask) |
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[7] << 4) & 0xFFFF);
            gCurrentFrameRenderData->viewport.rotations[gCurrentViewportIndex].m[3][1] =
                (D_801121E0[gCurrentViewportIndex].cameraTransform.rotation[8] << 20) & upperMask;
            gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex].m[3][2] =
                ((D_801121E0[gCurrentViewportIndex].cameraTransform.translation.x << 16) & upperMask) |
                (D_801121E0[gCurrentViewportIndex].cameraTransform.translation.y & 0xFFFF);
            gCurrentFrameRenderData->viewport.translations[gCurrentViewportIndex].m[3][3] =
                (D_801121E0[gCurrentViewportIndex].cameraTransform.translation.z << 16) & upperMask;

            if (gBackdropRenderCallbackList != NULL) {
                gSPPerspNormalize(
                    gRegionAllocPtr++,
                    gViewportStates[gCurrentViewportIndex].overlayPerspectiveNorm
                );
                gSPMatrix(
                    gRegionAllocPtr++,
                    &gCurrentFrameRenderData->viewport.overlayProjections[gCurrentViewportIndex],
                    G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION
                );
                gSPViewport(gRegionAllocPtr++, &gCurrentFrameRenderData->viewport.viewports[gCurrentViewportIndex]);
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
                gSPDisplayList(gRegionAllocPtr++, D_800DEF90);
                runRenderCallbacks(&gBackdropRenderCallbackList);
            }

            for (i = 0; i < 24; i += 3) {
                if (runtimeModelRenderCallbackLists[i] != NULL) {
                    hasModelCallbacks = 1;
                }
            }

            if (hasModelCallbacks != 0) {
                gSPPerspNormalize(gRegionAllocPtr++, gViewportStates[gCurrentViewportIndex].perspectiveNorm);
                gSPMatrix(
                    gRegionAllocPtr++,
                    &gCurrentFrameRenderData->viewport.projections[gCurrentViewportIndex],
                    G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION
                );
                gSPViewport(gRegionAllocPtr++, &gCurrentFrameRenderData->viewport.viewports[gCurrentViewportIndex]);
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
                gSPDisplayList(gRegionAllocPtr++, D_800DEF28);

                for (i = 0; i < 24; i += 3) {
                    if (runtimeModelRenderCallbackLists[i] != NULL) {
                        queue = &runtimeModelRenderCallbackLists[i];
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
            }

            if ((gRaceForegroundRenderCallbackList != NULL) || (gRaceOverlayRenderCallbackList != NULL)) {
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
                gSPDisplayList(gRegionAllocPtr++, D_800DF098);
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
        }
    }

    gMenuViewportWidth = 288;
    gMenuViewportHeight = 208;
    gMenuViewportCenterX = 160;
    gMenuViewportCenterY = 120;

    if ((gMenuForegroundRenderCallbackList != NULL) || (gMenuRenderCallbackList != NULL)) {
        gDPSetScissor(gRegionAllocPtr++, G_SC_NON_INTERLACE, 0, 0, 320, 240);
        gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
        if (gMenuRenderCallbackList != NULL) {
            runRenderCallbacks(&gMenuRenderCallbackList);
        }
        if (gMenuForegroundRenderCallbackList != NULL) {
            initMenuAsciiFontTexture();
            runRenderCallbacks(&gMenuForegroundRenderCallbackList);
        }
    }

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetScissor(gRegionAllocPtr++, G_SC_NON_INTERLACE, 0, 0, 320, 240);

    if (gMenuFadeAlpha != 0) {
        gSPDisplayList(gRegionAllocPtr++, D_800DF098);
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
}

#undef runtimeModelRenderCallbackLists
#undef VIEWPORT_COUNT
