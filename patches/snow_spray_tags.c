#include "patches.h"

#include "transform_ids.h"
#include "camera_interpolation.h"
#include "player_interpolation.h"
#include "game/race/items/race_item_effects.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/course/race_course_effects.h"
#include "game/race/camera/race_camera.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/system_runtime.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/race/player/race_player_input.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/asset_manager.h"
#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"

void getAssetTableImageAndExplicitPalette(u8 *, u16, u16, void **, void **);

#define ASSET_HANDLE(index) (gAssetHandles[index])
#define RACE_ITEM_GFX_CMD(pkt, cmd0, cmd1) \
    {                                      \
        Gfx *_g = (Gfx *)(pkt);            \
        _g->words.w0 = (cmd0);             \
        _g->words.w1 = (cmd1);             \
    }

static u16 nextSnowSpraySpawnId;

static void assignSnowSpraySpawnId(RaceItemFollowActor *actor) {
    u8 *bytes = (u8 *)actor;

    nextSnowSpraySpawnId++;
    bytes[0x52] = nextSnowSpraySpawnId >> 8;
    bytes[0x53] = nextSnowSpraySpawnId;
}

static u32 getSnowSpraySpawnId(RaceItemFollowActor *actor) {
    u8 *bytes = (u8 *)actor;
    return ((u32)bytes[0x52] << 8) | bytes[0x53];
}

static void pushSnowSprayMatrixGroup(RaceItemFollowActor *actor, u32 base, u32 side) {
    u32 id = base |
             ((getRacePlayerInterpolationGeneration() & MODELVIEW_SNOW_SPRAY_GENERATION_MASK)
              << MODELVIEW_SNOW_SPRAY_GENERATION_SHIFT) |
             ((u32)gCurrentViewportIndex << MODELVIEW_SNOW_SPRAY_VIEWPORT_SHIFT) |
             (getSnowSpraySpawnId(actor) << 1) | side;
    u32 component = actor->timer <= 1 || viewportCameraSkipsInterpolation()
                        ? G_EX_COMPONENT_SKIP : G_EX_COMPONENT_INTERPOLATE;
    gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                         component, component, component, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP,
                         G_EX_ORDER_LINEAR, G_EX_EDIT_NONE, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
}

RECOMP_PATCH void renderRacePlayerYellowTrickSparkles(RaceItemFollowActor *arg0) {
    Transform3D sp90;
    void *sp8C;
    void *sp88;
    volatile s32 pad[2];

    if (gRenderMatricesDirty != 0) {
        arg0->dirty = 1;
    }
    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos1) != 0) {
        if (arg0->dirty != 0) {
            arg0->dirty = 0;
            sp90 = gIdentityFixedTransform;
            sp90.translation.x = arg0->pos1.x;
            sp90.translation.y = arg0->pos1.y;
            sp90.translation.z = arg0->pos1.z;
            arg0->matrix1 = allocFixedTransformMatrix(&sp90);
            sp90.translation.x = arg0->pos2.x;
            sp90.translation.y = arg0->pos2.y;
            sp90.translation.z = arg0->pos2.z;
            arg0->matrix2 = allocFixedTransformMatrix(&sp90);
        }
        if (arg0->matrix2 != NULL) {
            getAssetTableImageAndPalette(
                getRelocatableHeapBlockBase(ASSET_HANDLE(0x1C)),
                (u16)((((s8)arg0->timer) >> 2) + 0x39),
                &sp8C,
                &sp88
            );

            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x06000000, (u32)gAlphaSpriteRenderModeDl);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xFD500000, (u32)sp8C);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF5500000, 0x07080200);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE6000000, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF3000000, 0x0703F800);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE7000000, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF5400200, 0x80200);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF2000000, 0x3C03C);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xFD100000, (u32)sp88);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE8000000, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF5000100, 0x07000000);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE6000000, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF0000000, 0x0703C000);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE7000000, 0);
            // @recomp Give the first sprite its own identity and snap it on replay teleports and camera cuts.
            pushSnowSprayMatrixGroup(arg0, MODELVIEW_SNOW_SPRAY_ID_BASE, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x01020040, (u32)arg0->matrix1);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x01000040, (u32)gViewportMatrix);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x0400103F, (u32)gRacePlayerYellowTrickSparklesQuadVertices);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xB1060402, 0x60200);
            // @recomp Pop this matrix group so subsequent draws cannot inherit this sprite's interpolation identity.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            // @recomp Match the second spray sprite separately with the same teleport and camera-cut handling.
            pushSnowSprayMatrixGroup(arg0, MODELVIEW_SNOW_SPRAY_ID_BASE, 1);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x01020040, (u32)arg0->matrix2);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x01000040, (u32)gViewportMatrix);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x0400103F, (u32)gRacePlayerYellowTrickSparklesQuadVertices);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xB1060402, 0x60200);
            // @recomp Pop this matrix group so subsequent draws cannot inherit this sprite's interpolation identity.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void renderRacePlayerBlueTrickSparkles(RaceItemFollowActor *arg0) {
    Transform3D sp98;
    void *sp94;
    void *sp90;
    volatile s32 pad[2];

    if (gRenderMatricesDirty != 0) {
        arg0->dirty = 1;
    }
    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos1) != 0) {
        if (arg0->dirty != 0) {
            arg0->dirty = 0;
            sp98 = gIdentityFixedTransform;
            sp98.translation.x = arg0->pos1.x;
            sp98.translation.y = arg0->pos1.y;
            sp98.translation.z = arg0->pos1.z;
            arg0->matrix1 = allocFixedTransformMatrix(&sp98);
            sp98.translation.x = arg0->pos2.x;
            sp98.translation.y = arg0->pos2.y;
            sp98.translation.z = arg0->pos2.z;
            arg0->matrix2 = allocFixedTransformMatrix(&sp98);
        }
        if (arg0->matrix2 != NULL) {
            getAssetTableImageAndExplicitPalette(
                (u8 *)getRelocatableHeapBlockBase(ASSET_HANDLE(0x1C)),
                (u16)((((s8)arg0->timer) >> 2) + 0x39),
                0x12,
                &sp94,
                &sp90
            );

            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x06000000, (u32)gAlphaSpriteRenderModeDl);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xFD500000, (u32)sp94);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF5500000, 0x07080200);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE6000000, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF3000000, 0x0703F800);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE7000000, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF5400200, 0x80200);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF2000000, 0x3C03C);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xFD100000, (u32)sp90);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE8000000, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF5000100, 0x07000000);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE6000000, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xF0000000, 0x0703C000);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xE7000000, 0);
            // @recomp Give the first sprite its own identity and snap it on replay teleports and camera cuts.
            pushSnowSprayMatrixGroup(arg0, MODELVIEW_LANDING_SNOW_SPRAY_ID_BASE, 0);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x01020040, (u32)arg0->matrix1);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x01000040, (u32)gViewportMatrix);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x0400103F, (u32)gRacePlayerBlueTrickSparklesQuadVertices);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xB1060402, 0x60200);
            // @recomp Pop this matrix group so subsequent draws cannot inherit this sprite's interpolation identity.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            // @recomp Match the second spray sprite separately with the same teleport and camera-cut handling.
            pushSnowSprayMatrixGroup(arg0, MODELVIEW_LANDING_SNOW_SPRAY_ID_BASE, 1);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x01020040, (u32)arg0->matrix2);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x01000040, (u32)gViewportMatrix);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0x0400103F, (u32)gRacePlayerBlueTrickSparklesQuadVertices);
            RACE_ITEM_GFX_CMD(gRegionAllocPtr++, 0xB1060402, 0x60200);
            // @recomp Pop this matrix group so subsequent draws cannot inherit this sprite's interpolation identity.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void initRacePlayerYellowTrickSparkles(RaceItemFollowActor *arg0) {
    RacePlayer *player;

    // @recomp Give each actor a unique id
    assignSnowSpraySpawnId(arg0);
    arg0->timer = -1;
    player = &gRacePlayers[arg0->task.userId];
    if (player->stateFlags & 0x400) {
        arg0->offset1.x = player->groundMarkerSources[0].x - player->unk28.x;
        arg0->offset1.y = player->groundMarkerSources[0].y - player->unk28.y;
        arg0->offset1.z = player->groundMarkerSources[0].z - player->unk28.z;
        arg0->offset2.x = player->groundMarkerSources[2].x - player->unk28.x;
        arg0->offset2.y = player->groundMarkerSources[2].y - player->unk28.y;
        arg0->offset2.z = player->groundMarkerSources[2].z - player->unk28.z;
    } else {
        arg0->offset1.x = player->groundMarkerSources[1].x - player->unk28.x;
        arg0->offset1.y = player->groundMarkerSources[1].y - player->unk28.y;
        arg0->offset1.z = player->groundMarkerSources[1].z - player->unk28.z;
        arg0->offset2.x = player->groundMarkerSources[3].x - player->unk28.x;
        arg0->offset2.y = player->groundMarkerSources[3].y - player->unk28.y;
        arg0->offset2.z = player->groundMarkerSources[3].z - player->unk28.z;
    }
    updateRacePlayerYellowTrickSparkles(arg0);
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRacePlayerYellowTrickSparkles);
}

RECOMP_PATCH void initRacePlayerBlueTrickSparkles(RaceItemFollowActor *arg0) {
    RacePlayer *player;

    // @recomp Give each actor a unique id
    assignSnowSpraySpawnId(arg0);
    arg0->timer = -1;
    player = &gRacePlayers[arg0->task.userId];
    if (player->stateFlags & 0x400) {
        arg0->offset1.x = player->groundMarkerSources[0].x - player->unk28.x;
        arg0->offset1.y = player->groundMarkerSources[0].y - player->unk28.y;
        arg0->offset1.z = player->groundMarkerSources[0].z - player->unk28.z;
        arg0->offset2.x = player->groundMarkerSources[2].x - player->unk28.x;
        arg0->offset2.y = player->groundMarkerSources[2].y - player->unk28.y;
        arg0->offset2.z = player->groundMarkerSources[2].z - player->unk28.z;
    } else {
        arg0->offset1.x = player->groundMarkerSources[1].x - player->unk28.x;
        arg0->offset1.y = player->groundMarkerSources[1].y - player->unk28.y;
        arg0->offset1.z = player->groundMarkerSources[1].z - player->unk28.z;
        arg0->offset2.x = player->groundMarkerSources[3].x - player->unk28.x;
        arg0->offset2.y = player->groundMarkerSources[3].y - player->unk28.y;
        arg0->offset2.z = player->groundMarkerSources[3].z - player->unk28.z;
    }
    updateRacePlayerBlueTrickSparkles(arg0);
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRacePlayerBlueTrickSparkles);
}

static u16 nextGroundSpraySpawnId;

RECOMP_PATCH void initRaceItemSparkBurst(RaceItemSparkBurstActor *arg0) {
    // @recomp Store each node's lifetime ID in two tail-padding bytes
    for (s32 i = 0; i < 2; i++) {
        u8 *spawnId = (u8 *)&arg0->drawNodes[i] + __builtin_offsetof(RaceItemDrawNode, matrixDirty) + 1;
        nextGroundSpraySpawnId++;
        spawnId[0] = nextGroundSpraySpawnId >> 8;
        spawnId[1] = nextGroundSpraySpawnId;
    }
    arg0->timer = 0;
    // @recomp Defer draw-list insertion until the next task update so new puffs do not lead the interpolated board.
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceItemSparkBurst);
}

RECOMP_PATCH void renderRaceItemTextureEffects(RaceItemTextureActor *arg0) {
    register RaceItemTextureActor *actor;
    s32 i;
    RaceItemDrawNode *node;
    Transform3D transform;

    actor = arg0;
    transform = gIdentityFixedTransform;
    do {
        if (gRenderMatricesDirty != 0) {
            i = 0;
            do {
                node = gRaceItemTextureEffectDrawLists[i++];
                if (node != NULL) {
                    do {
                        node->matrixDirty = 1;
                        node = node->next;
                    } while (node != NULL);
                }
            } while ((void *)gRaceCameras != &gRaceItemTextureEffectDrawLists[i]);
        }
        gSPDisplayList(gRegionAllocPtr++, gEffectRenderModeSetupDl);
        i = 0;
        do {
            node = gRaceItemTextureEffectDrawLists[i];
            if (node != NULL) {
                gDPLoadTextureBlock_4b(
                    gRegionAllocPtr++,
                    actor->images[i],
                    G_IM_FMT_CI,
                    16,
                    16,
                    0,
                    G_TX_CLAMP,
                    G_TX_CLAMP,
                    G_TX_NOMASK,
                    G_TX_NOMASK,
                    G_TX_NOLOD,
                    G_TX_NOLOD
                );
                gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, actor->palettes[i]);
            }
            if (node != NULL) {
                do {
                    if (isPositionNearCurrentRaceViewportCamera(node->pos) != 0) {
                        if (node->matrixDirty != 0) {
                            node->matrixDirty = 0;
                            transform.translation.x = node->pos->x;
                            transform.translation.y = node->pos->y;
                            transform.translation.z = node->pos->z;
                            node->matrix = allocFixedTransformMatrix(&transform);
                        }
                        // @recomp Match each ground-contact puff by its spawn ID, independent of draw-list order.
                        u8 *spawnId = (u8 *)node + __builtin_offsetof(RaceItemDrawNode, matrixDirty) + 1;
                        u32 id = MODELVIEW_GROUND_SPRAY_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_GROUND_SPRAY_VIEWPORT_SHIFT) |
                                 ((u32)spawnId[0] << 8) | spawnId[1];
                        u32 component = viewportCameraSkipsInterpolation()
                                            ? G_EX_COMPONENT_SKIP : G_EX_COMPONENT_INTERPOLATE;
                        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                                             component, component, component, G_EX_COMPONENT_SKIP,
                                             G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
                        gSPMatrix(gRegionAllocPtr++, node->matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                        gSPMatrix(gRegionAllocPtr++, gViewportMatrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
                        gSPVertex(gRegionAllocPtr++, node->vertices, 4, 0);
                        gSP2Triangles(gRegionAllocPtr++, 3, 2, 1, 0, 3, 1, 0, 0);
                        // @recomp End this puff's group before drawing the next independently spawned particle.
                        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
                    }
                    node = node->next;
                } while (node != NULL);
            }
            i++;
        } while ((void *)gRaceCameras != &gRaceItemTextureEffectDrawLists[i]);
        gSPDisplayList(gRegionAllocPtr++, gEffectRenderModeCleanupDl);
    } while (0);
}
