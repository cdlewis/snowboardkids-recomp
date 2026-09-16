#include "patches.h"

#include "transform_ids.h"
#include "camera_interpolation.h"
#include "game/race/course/race_course_props_and_pickups.h"
#include "game/race/course/race_course_effects.h"
#include "game/race/race_state.h"
#include "game/race/camera/race_camera.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"

extern Gfx gRaceItemPickupDisplayList[];
extern Gfx gRaceActionPickupDisplayList[];
extern Vtx gRacePickupBaseVertices[];
extern Vtx gRacePickupTopVertices[];
extern Vec3i gPickupShardInitialVelocities[];
extern s16 gFrameCounter;

typedef struct {
    Transform3D source;
    s32 pad20;
} RacePickupMatrixScratch;

#define RACE_PICKUP_G_TRI2 0xB1
#define racePickupTriangleWord(v0, v1, v2, flag) \
    (_SHIFTL((flag), 24, 8) | _SHIFTL((v0) * 2, 16, 8) | _SHIFTL((v1) * 2, 8, 8) | _SHIFTL((v2) * 2, 0, 8))
#define gRacePickupQuadrangle(pkt, v0, v1, v2, v3, flag)                                                \
    {                                                                                                   \
        Gfx *_g = (Gfx *)(pkt);                                                                         \
        _g->words.w0 = (_SHIFTL(RACE_PICKUP_G_TRI2, 24, 8) | racePickupTriangleWord(v0, v1, v2, flag)); \
        _g->words.w1 = racePickupTriangleWord(v0, v2, v3, flag);                                        \
    }

static u16 nextPickupShardSpawnId;

static void pushRacePickupMatrixGroup(RacePickupActor *actor, u32 part) {
    u32 id = MODELVIEW_RACE_PICKUP_ID_BASE | actor->spawnIndex | (part << 16) |
             ((u32)gRaceCourseIndex.signedValue << 18) | ((u32)gCurrentViewportIndex << 22);
    u32 component = viewportCameraSkipsInterpolation() ? G_EX_COMPONENT_SKIP : G_EX_COMPONENT_INTERPOLATE;
    gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                         component, component, component, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP,
                         G_EX_ORDER_LINEAR, G_EX_EDIT_NONE, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
}

RECOMP_PATCH void renderRacePickupIdle(RacePickupActor *arg0) {
    RacePickupMatrixScratch spF4;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            spF4.source = gIdentityFixedTransform;
            spF4.source.translation.x = arg0->drawPos.x;
            spF4.source.translation.y = arg0->drawPos.y;
            spF4.source.translation.z = arg0->drawPos.z;
            arg0->displayList = allocFixedTransformMatrix(&spF4.source);
            arg0->rotationDisplayList = allocFixedTransformMatrix(&arg0->transform);
            spF4.source = arg0->transform;
            spF4.source.rotation[0] /= 2;
            spF4.source.rotation[1] /= 2;
            spF4.source.rotation[2] /= 2;
            spF4.source.rotation[3] /= 2;
            spF4.source.rotation[4] /= 2;
            spF4.source.rotation[5] /= 2;
            spF4.source.rotation[6] /= 2;
            spF4.source.rotation[7] /= 2;
            spF4.source.rotation[8] /= 2;
            spF4.source.translation.y += (fixedSine((s16)((gFrameCounter << 7) & 0xFFF)) << 7) + 0x300000;
            arg0->scaleDisplayList = allocFixedTransformMatrix(&spF4.source);
        }

        if (arg0->scaleDisplayList != NULL) {
            gDPPipeSync(gRegionAllocPtr++);
            gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(gAssetHandles[0xA]));
            gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(gAssetHandles[0xB]));
            // @recomp Match this shop part by spawn and viewport.
            pushRacePickupMatrixGroup(arg0, 2);
            gSPMatrix(gRegionAllocPtr++, arg0->scaleDisplayList, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            if (arg0->variant == 0) {
                gSPDisplayList(gRegionAllocPtr++, gRaceItemPickupDisplayList);
            } else {
                gSPDisplayList(gRegionAllocPtr++, gRaceActionPickupDisplayList);
            }
            gSPDisplayList(gRegionAllocPtr++, gEffectRenderModeSetupDl);
            gDPLoadTextureBlock_4b(
                gRegionAllocPtr++,
                arg0->image0,
                G_IM_FMT_CI,
                32,
                32,
                0,
                G_TX_CLAMP,
                G_TX_CLAMP,
                0,
                0,
                0,
                0
            );
            gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->palette0);
            // @recomp Close the preceding part before loading this part's transform.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            // @recomp Match this shop part by spawn and viewport.
            pushRacePickupMatrixGroup(arg0, 0);
            gSPMatrix(gRegionAllocPtr++, arg0->displayList, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPMatrix(gRegionAllocPtr++, gViewportMatrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
            gDma1p(gRegionAllocPtr++, G_VTX, gRacePickupBaseVertices, 0x207F, 0);
            gRacePickupQuadrangle(gRegionAllocPtr++, 3, 2, 1, 0, 0);
            gDPLoadTextureBlock_4b(
                gRegionAllocPtr++,
                arg0->image1,
                G_IM_FMT_CI,
                32,
                32,
                0,
                G_TX_CLAMP,
                G_TX_CLAMP,
                0,
                0,
                0,
                0
            );
            gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->palette1);
            // @recomp Close the preceding part before loading this part's transform.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            // @recomp Match this shop part by spawn and viewport.
            pushRacePickupMatrixGroup(arg0, 1);
            gSPMatrix(gRegionAllocPtr++, arg0->rotationDisplayList, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gDma1p(gRegionAllocPtr++, G_VTX, gRacePickupTopVertices, 0x513F, 0);
            gRacePickupQuadrangle(gRegionAllocPtr++, 3, 2, 1, 0, 0);
            gRacePickupQuadrangle(gRegionAllocPtr++, 7, 6, 5, 4, 0);
            gRacePickupQuadrangle(gRegionAllocPtr++, 11, 10, 9, 8, 0);
            gRacePickupQuadrangle(gRegionAllocPtr++, 15, 14, 13, 12, 0);
            gDPLoadTextureBlock_4b(
                gRegionAllocPtr++,
                arg0->image2,
                G_IM_FMT_CI,
                32,
                32,
                0,
                G_TX_CLAMP,
                G_TX_CLAMP,
                0,
                0,
                0,
                0
            );
            gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->palette2);
            gRacePickupQuadrangle(gRegionAllocPtr++, 19, 18, 17, 16, 0);
            gSPDisplayList(gRegionAllocPtr++, gEffectRenderModeCleanupDl);
            // @recomp End the shell identity before another actor is rendered.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void renderRacePickupBase(RacePickupActor *arg0) {
    RacePickupMatrixScratch sp64;
    Gfx *temp_v0;
    Gfx *temp_v0_2;
    Gfx *temp_v0_3;
    Gfx *temp_v0_4;
    Gfx *temp_v0_5;
    Gfx *temp_v0_6;
    Gfx *temp_v0_7;
    Gfx *temp_v0_8;
    Gfx *temp_v0_9;
    Gfx *temp_v0_10;
    Gfx *temp_v0_11;
    Gfx *temp_v0_12;
    Gfx *temp_v0_13;
    Gfx *temp_v0_14;
    Gfx *temp_v0_15;
    Gfx *temp_v0_16;
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;
    Gfx *temp_v0_19;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            sp64.source = gIdentityFixedTransform;
            sp64.source.translation.x = arg0->drawPos.x;
            sp64.source.translation.y = arg0->drawPos.y;
            sp64.source.translation.z = arg0->drawPos.z;
            arg0->displayList = allocFixedTransformMatrix(&sp64.source);
        }
        // @recomp Keep the same tag for the part of the shop that remains after the box breaks.
        pushRacePickupMatrixGroup(arg0, 0);
        do { if (arg0->displayList != NULL) { temp_v0 = gRegionAllocPtr++; temp_v0->words.w0 = 0x06000000; temp_v0->words.w1 = (u32) gEffectRenderModeSetupDl; temp_v0_2 = gRegionAllocPtr++; temp_v0_2->words.w0 = 0xFD500000; temp_v0_2->words.w1 = (u32) arg0->image0; temp_v0_3 = gRegionAllocPtr++; temp_v0_3->words.w0 = 0xF5500000; temp_v0_3->words.w1 = 0x07080200; temp_v0_4 = gRegionAllocPtr++; temp_v0_4->words.w1 = 0; temp_v0_4->words.w0 = 0xE6000000; temp_v0_5 = gRegionAllocPtr++; temp_v0_5->words.w0 = 0xF3000000; temp_v0_5->words.w1 = 0x070FF400; temp_v0_6 = gRegionAllocPtr++; temp_v0_6->words.w1 = 0; temp_v0_6->words.w0 = 0xE7000000; temp_v0_7 = gRegionAllocPtr++; temp_v0_7->words.w0 = 0xF5400400; temp_v0_7->words.w1 = 0x00080200; temp_v0_8 = gRegionAllocPtr++; temp_v0_8->words.w0 = 0xF2000000; temp_v0_8->words.w1 = 0x0007C07C; temp_v0_9 = gRegionAllocPtr++; temp_v0_9->words.w0 = 0xFD100000; temp_v0_9->words.w1 = (u32) arg0->palette0; temp_v0_10 = gRegionAllocPtr++; temp_v0_10->words.w1 = 0; temp_v0_10->words.w0 = 0xE8000000; temp_v0_11 = gRegionAllocPtr++; temp_v0_11->words.w0 = 0xF5000100; temp_v0_11->words.w1 = 0x07000000; temp_v0_12 = gRegionAllocPtr++; temp_v0_12->words.w1 = 0; temp_v0_12->words.w0 = 0xE6000000; temp_v0_13 = gRegionAllocPtr++; temp_v0_13->words.w0 = 0xF0000000; temp_v0_13->words.w1 = 0x0703C000; temp_v0_14 = gRegionAllocPtr++; temp_v0_14->words.w1 = 0; temp_v0_14->words.w0 = 0xE7000000; temp_v0_15 = gRegionAllocPtr++; temp_v0_15->words.w0 = 0x01020040; temp_v0_15->words.w1 = (u32) arg0->displayList; temp_v0_16 = gRegionAllocPtr++; temp_v0_16->words.w0 = 0x01000040; temp_v0_16->words.w1 = (u32) gViewportMatrix; temp_v0_17 = gRegionAllocPtr++; temp_v0_17->words.w0 = 0x0400207F; temp_v0_17->words.w1 = (u32) gRacePickupBaseVertices; temp_v0_18 = gRegionAllocPtr++; temp_v0_18->words.w0 = 0xB1060402; temp_v0_18->words.w1 = 0x00060200; temp_v0_19 = gRegionAllocPtr++; temp_v0_19->words.w0 = 0x06000000; temp_v0_19->words.w1 = (u32) gEffectRenderModeCleanupDl; } } while (0);
        // @recomp Do not let this shop identity leak into the following actor.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderRacePickupRespawn(RacePickupActor *arg0) {
    RacePickupMatrixScratch spF4;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }
    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            spF4.source = gIdentityFixedTransform;
            spF4.source.translation.x = arg0->drawPos.x;
            spF4.source.translation.y = arg0->drawPos.y;
            spF4.source.translation.z = arg0->drawPos.z;
            arg0->displayList = allocFixedTransformMatrix(&spF4.source);
            arg0->rotationDisplayList = allocFixedTransformMatrix(&arg0->transform);
        }
        if (arg0->displayList != NULL) {
            if (arg0->rotationDisplayList != NULL) {
                gSPDisplayList(gRegionAllocPtr++, gEffectRenderModeSetupDl);
                gDPLoadTextureBlock_4b(
                    gRegionAllocPtr++,
                    arg0->image0,
                    G_IM_FMT_CI,
                    32,
                    32,
                    0,
                    G_TX_CLAMP,
                    G_TX_CLAMP,
                    0,
                    0,
                    0,
                    0
                );
                gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->palette0);
                // @recomp Match this shop part by spawn and viewport.
                pushRacePickupMatrixGroup(arg0, 0);
                gSPMatrix(gRegionAllocPtr++, arg0->displayList, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPMatrix(gRegionAllocPtr++, gViewportMatrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
                gDma1p(gRegionAllocPtr++, G_VTX, gRacePickupBaseVertices, 0x103F, 0);
                gRacePickupQuadrangle(gRegionAllocPtr++, 3, 2, 1, 0, 0);
                gDPLoadTextureBlock_4b(
                    gRegionAllocPtr++,
                    arg0->image1,
                    G_IM_FMT_CI,
                    32,
                    32,
                    0,
                    G_TX_CLAMP,
                    G_TX_CLAMP,
                    0,
                    0,
                    0,
                    0
                );
                gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->palette1);
                // @recomp Close the preceding part before loading this part's transform.
                gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
                // @recomp Match this shop part by spawn and viewport.
                pushRacePickupMatrixGroup(arg0, 1);
                gSPMatrix(gRegionAllocPtr++, arg0->rotationDisplayList, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gDma1p(gRegionAllocPtr++, G_VTX, gRacePickupTopVertices, 0x513F, 0);
                gRacePickupQuadrangle(gRegionAllocPtr++, 3, 2, 1, 0, 0);
                gRacePickupQuadrangle(gRegionAllocPtr++, 7, 6, 5, 4, 0);
                gRacePickupQuadrangle(gRegionAllocPtr++, 11, 10, 9, 8, 0);
                gRacePickupQuadrangle(gRegionAllocPtr++, 15, 14, 13, 12, 0);
                gDPLoadTextureBlock_4b(
                    gRegionAllocPtr++,
                    arg0->image2,
                    G_IM_FMT_CI,
                    32,
                    32,
                    0,
                    G_TX_CLAMP,
                    G_TX_CLAMP,
                    0,
                    0,
                    0,
                    0
                );
                gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->palette2);
                gRacePickupQuadrangle(gRegionAllocPtr++, 19, 18, 17, 16, 0);
                gSPDisplayList(gRegionAllocPtr++, gEffectRenderModeCleanupDl);
                // @recomp End the shell identity before another actor is rendered.
                gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            }
        }
    }
}

RECOMP_PATCH void renderPickupShardParticle(PickupShardParticleActor *arg0) {
    volatile s32 pad;
    Transform3D transform;
    Gfx *temp_v0;
    Gfx *temp_v0_10;
    Gfx *temp_v0_11;
    Gfx *temp_v0_12;
    Gfx *temp_v0_13;
    Gfx *temp_v0_14;
    Gfx *temp_v0_15;
    Gfx *temp_v0_16;
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;
    Gfx *temp_v0_19;
    Gfx *temp_v0_2;
    Gfx *temp_v0_3;
    Gfx *temp_v0_4;
    Gfx *temp_v0_5;
    Gfx *temp_v0_6;
    Gfx *temp_v0_7;
    Gfx *temp_v0_8;
    Gfx *temp_v0_9;
    Gfx *var_v0;

    if (gRenderMatricesDirty != 0) {
        arg0->transformDirty = 1;
    }
    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->transformDirty != 0) {
            arg0->transformDirty = 0;
            makeFixedRotationXYZ(transform.rotation, arg0->rotX, arg0->rotY, arg0->rotZ);
            transform.translation.x = arg0->pos.x;
            transform.translation.y = arg0->pos.y;
            transform.translation.z = arg0->pos.z;
            arg0->displayList = allocFixedTransformMatrix(&transform);
        }
        if (arg0->displayList != NULL) {
            // @recomp The unused bytes at 0x3E hold a spawn serial, so recycled task slots cannot match old debris.
            u32 id = MODELVIEW_PICKUP_SHARD_ID_BASE | ((u32)gCurrentViewportIndex << 22) |
                     ((u32)(u8)arg0->pad3E[0] << 8) | (u8)arg0->pad3E[1];
            u32 component = viewportCameraSkipsInterpolation() ? G_EX_COMPONENT_SKIP : G_EX_COMPONENT_INTERPOLATE;
            // @recomp Match each flying shard by computed identity.
            gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                                 component, component, component, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP,
                                 G_EX_ORDER_LINEAR, G_EX_EDIT_NONE, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
            temp_v0 = gRegionAllocPtr++;
            temp_v0->words.w1 = (u32)gEffectRenderModeSetupDl;
            temp_v0->words.w0 = 0x06000000;
            temp_v0_2 = gRegionAllocPtr++; temp_v0_2->words.w0 = 0xFD500000; temp_v0_2->words.w1 = (u32)arg0->palette; temp_v0_3 = gRegionAllocPtr++; temp_v0_3->words.w0 = 0xF5500000; temp_v0_3->words.w1 = 0x07080200; temp_v0_4 = gRegionAllocPtr++; temp_v0_4->words.w1 = 0; temp_v0_4->words.w0 = 0xE6000000; temp_v0_5 = gRegionAllocPtr++; temp_v0_5->words.w0 = 0xF3000000; temp_v0_5->words.w1 = 0x070FF400; temp_v0_6 = gRegionAllocPtr++; temp_v0_6->words.w1 = 0; temp_v0_6->words.w0 = 0xE7000000; temp_v0_7 = gRegionAllocPtr++; temp_v0_7->words.w0 = 0xF5400400; temp_v0_7->words.w1 = 0x00080200; temp_v0_8 = gRegionAllocPtr++; temp_v0_8->words.w0 = 0xF2000000; temp_v0_8->words.w1 = 0x0007C07C; temp_v0_9 = gRegionAllocPtr++; temp_v0_9->words.w0 = 0xFD100000; temp_v0_9->words.w1 = (u32)arg0->image; temp_v0_10 = gRegionAllocPtr++; temp_v0_10->words.w1 = 0; temp_v0_10->words.w0 = 0xE8000000; temp_v0_11 = gRegionAllocPtr++; temp_v0_11->words.w0 = 0xF5000100; temp_v0_11->words.w1 = 0x07000000; temp_v0_12 = gRegionAllocPtr++; temp_v0_12->words.w1 = 0; temp_v0_12->words.w0 = 0xE6000000; temp_v0_13 = gRegionAllocPtr++; temp_v0_13->words.w0 = 0xF0000000; temp_v0_13->words.w1 = 0x0703C000; temp_v0_14 = gRegionAllocPtr++; temp_v0_14->words.w1 = 0; temp_v0_14->words.w0 = 0xE7000000; temp_v0_15 = gRegionAllocPtr++; temp_v0_15->words.w0 = 0x01020040; temp_v0_15->words.w1 = (u32)arg0->displayList; temp_v0_16 = gRegionAllocPtr++; temp_v0_16->words.w0 = 0x0400103F; temp_v0_16->words.w1 = (u32)&gRacePickupBaseVertices[((((u16)arg0->spawnOffsetIndex) >> 1) * 4) + 8]; if (arg0->spawnOffsetIndex & 1) { temp_v0_17 = gRegionAllocPtr++;
                temp_v0_17->words.w1 = 0x604;
                temp_v0_17->words.w0 = 0xBF000000;
                var_v0 = gRegionAllocPtr++;
                var_v0->words.w1 = 0x406;
                var_v0->words.w0 = 0xBF000000;
            } else {
                temp_v0_18 = gRegionAllocPtr++;
                temp_v0_18->words.w1 = 0x402;
                temp_v0_18->words.w0 = 0xBF000000;
                var_v0 = gRegionAllocPtr++;
                var_v0->words.w1 = 0x204;
                var_v0->words.w0 = 0xBF000000;
            }
            temp_v0_19 = gRegionAllocPtr++;
            temp_v0_19->words.w1 = (u32)gEffectRenderModeCleanupDl;
            temp_v0_19->words.w0 = 0x06000000;
            // @recomp End the shard identity before another particle is rendered.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void initPickupShardParticle(PickupShardParticleActor *arg0) {
    Transform3D transform;

    // @recomp Reserve unused actor padding for a new spawn ID
    nextPickupShardSpawnId++;
    arg0->pad3E[0] = nextPickupShardSpawnId >> 8;
    arg0->pad3E[1] = nextPickupShardSpawnId;

    arg0->timer = 0xA;
    arg0->rotVelX = randomNextMain() - 0x80;
    arg0->rotVelY = randomNextMain() - 0x80;
    arg0->rotVelZ = randomNextMain() - 0x80;
    makeFixedRotationY(transform.rotation, arg0->rotY);
    transformVec3iByFixedMatrix(
        transform.rotation,
        &gPickupShardInitialVelocities[arg0->spawnOffsetIndex],
        &arg0->velocity
    );
    getAssetTableImageAndPalette(getRelocatableHeapBlockBase(gAssetHandles[0x1C]), 0x22, &arg0->palette, &arg0->image);
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updatePickupShardParticle);
}
