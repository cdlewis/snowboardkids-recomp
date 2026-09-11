#include "patches.h"

#include "transform_ids.h"
#include "camera_interpolation.h"
#include "player_interpolation.h"

#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/math/spatial_math.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/player/race_player_model_renderer.h"

extern s16 gUiBlinkTimer;
extern Gfx *gRegionAllocPtr;
extern u8 gRenderMatricesDirty;
extern u8 gCurrentViewportIndex;

extern Gfx gRacePlayerShadowRenderSetupDisplayList[];



static Gfx **const gRacePlayerModelPartDisplayLists[] = {
    gRacePlayerModelPart0DisplayLists,  gRacePlayerModelPart1DisplayLists,  gRacePlayerModelPart2DisplayLists,
    gRacePlayerModelPart3DisplayLists,  gRacePlayerModelPart4DisplayLists,  gRacePlayerModelPart5DisplayLists,
    gRacePlayerModelPart6DisplayLists,  gRacePlayerModelPart7DisplayLists,  gRacePlayerModelPart8DisplayLists,
    gRacePlayerModelPart9DisplayLists,  gRacePlayerModelPart10DisplayLists, gRacePlayerModelPart11DisplayLists,
    gRacePlayerModelPart12DisplayLists,
};

static Gfx **const gRaceGhostPlayerModelPartDisplayLists[] = {
    gRaceGhostPlayerModelPart0DisplayLists,  gRaceGhostPlayerModelPart1DisplayLists,
    gRaceGhostPlayerModelPart2DisplayLists,  gRaceGhostPlayerModelPart3DisplayLists,
    gRaceGhostPlayerModelPart4DisplayLists,  gRaceGhostPlayerModelPart5DisplayLists,
    gRaceGhostPlayerModelPart6DisplayLists,  gRaceGhostPlayerModelPart7DisplayLists,
    gRaceGhostPlayerModelPart8DisplayLists,  gRaceGhostPlayerModelPart9DisplayLists,
    gRaceGhostPlayerModelPart10DisplayLists, gRaceGhostPlayerModelPart11DisplayLists,
    gRaceGhostPlayerModelPart12DisplayLists,
};

static u32 sRacePlayerInterpolationGeneration;

void invalidateRacePlayerInterpolation(void) {
    // @recomp Change all player, board and shadow identities when replay snapshots replace their transforms.
    sRacePlayerInterpolationGeneration =
        (sRacePlayerInterpolationGeneration + 1) & MODELVIEW_RACE_PLAYER_GENERATION_MASK;
}

static u32 getRacePlayerMatrixGroupId(u32 base, u16 playerIndex, s32 boneIndex) {
    return base |
           (sRacePlayerInterpolationGeneration << MODELVIEW_RACE_PLAYER_GENERATION_SHIFT) |
           ((u32)(gCurrentViewportIndex & MODELVIEW_RACE_PLAYER_VIEWPORT_MASK)
            << MODELVIEW_RACE_PLAYER_VIEWPORT_SHIFT) |
           ((u32)(playerIndex & MODELVIEW_RACE_PLAYER_INDEX_MASK) << MODELVIEW_RACE_PLAYER_INDEX_SHIFT) |
           ((u32)boneIndex & MODELVIEW_RACE_PLAYER_BONE_MASK);
}

static void pushRacePlayerBoneMatrixGroup(u16 playerIndex, s32 boneIndex) {
    // @recomp Snap player transforms with camera cuts instead of blending the old pose into the new shot.
    u32 component = viewportCameraSkipsInterpolation() ? G_EX_COMPONENT_SKIP : G_EX_COMPONENT_INTERPOLATE;
    gEXMatrixGroupSimple(
        gRegionAllocPtr++,
        getRacePlayerMatrixGroupId(MODELVIEW_RACE_PLAYER_BONE_ID_BASE, playerIndex, boneIndex),
        G_EX_PUSH,
        G_MTX_MODELVIEW,
        component,
        component,
        G_EX_COMPONENT_SKIP,
        G_EX_COMPONENT_SKIP,
        G_EX_COMPONENT_AUTO,
        G_EX_ORDER_AUTO,
        G_EX_EDIT_NONE,
        G_EX_COMPONENT_AUTO,
        G_EX_COMPONENT_AUTO
    );
}

static void pushRacePlayerShadowMatrixGroup(u16 playerIndex) {
    // @recomp Snap player transforms with camera cuts instead of blending the old pose into the new shot.
    u32 component = viewportCameraSkipsInterpolation() ? G_EX_COMPONENT_SKIP : G_EX_COMPONENT_INTERPOLATE;
    gEXMatrixGroupSimple(
        gRegionAllocPtr++,
        getRacePlayerMatrixGroupId(MODELVIEW_RACE_PLAYER_SHADOW_ID_BASE, playerIndex, 0),
        G_EX_PUSH,
        G_MTX_MODELVIEW,
        component,
        component,
        G_EX_COMPONENT_SKIP,
        component,
        G_EX_COMPONENT_AUTO,
        G_EX_ORDER_AUTO,
        G_EX_EDIT_NONE,
        G_EX_COMPONENT_AUTO,
        G_EX_COMPONENT_AUTO
    );
}

static void popRacePlayerMatrixGroup(void) {
    gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
}

RECOMP_PATCH void drawRacePlayerGroundShadow(RacePlayer *player) {
    s32 i;

    if (gRenderMatricesDirty != 0) {
        player->stateFlags &= ~RACE_PLAYER_MODEL_RENDERER_FLAG_SHADOW_READY;
        player->shadowVtx = allocMenuRenderScratch(0x40);
        if (player->shadowVtx == NULL) {
            return;
        }

        for (i = 0; i < RACE_PLAYER_SHADOW_VERTEX_COUNT; i++) {
            player->shadowVtx[i].v.ob[0] = (player->markerPoints[i].x - player->markerPoints[0].x) >> 14;
            player->shadowVtx[i].v.ob[1] = (player->markerPoints[i].y - player->markerPoints[0].y) >> 14;
            player->shadowVtx[i].v.ob[2] = (player->markerPoints[i].z - player->markerPoints[0].z) >> 14;
            player->shadowVtx[i].v.flag = 0;
            player->shadowVtx[i].v.cn[0] = 0;
            player->shadowVtx[i].v.cn[1] = 0;
            player->shadowVtx[i].v.cn[2] = 0;
            player->shadowVtx[i].v.cn[3] = 0x30;
        }

        player->shadowMtx = allocMenuRenderScratch(0x100);
        if (player->shadowMtx == NULL) {
            return;
        }

        *player->shadowMtx = gRacePlayerShadowMatrixTemplate;

        player->shadowMtx->m[1][2] =
            (player->markerPoints[0].x & 0xFFFF0000) | (((player->markerPoints[0].y + 0xA000) >> 16) & 0xFFFF);
        player->shadowMtx->m[1][3] = (player->markerPoints[0].z & 0xFFFF0000) | 1;
        player->shadowMtx->m[3][2] =
            ((player->markerPoints[0].x << 16) & 0xFFFF0000) | ((player->markerPoints[0].y + 0xA000) & 0xFFFF);
        player->shadowMtx->m[3][3] = (player->markerPoints[0].z << 16) & 0xFFFF0000;
        player->stateFlags |= RACE_PLAYER_MODEL_RENDERER_FLAG_SHADOW_READY;
    }

    if (isPositionNearCurrentRaceViewportCamera(player->markerPoints) != 0 &&
        (player->stateFlags & RACE_PLAYER_MODEL_RENDERER_FLAG_SHADOW_READY) != 0) {
        gSPDisplayList(gRegionAllocPtr++, gRacePlayerShadowRenderSetupDisplayList);

        // @recomp Give the shadow a stable matrix identity across interpolated frames.
        pushRacePlayerShadowMatrixGroup(player->playerIndex);

        gSPMatrix(gRegionAllocPtr++, player->shadowMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPVertex(gRegionAllocPtr++, player->shadowVtx, RACE_PLAYER_SHADOW_VERTEX_COUNT, 0);
        gSP1Quadrangle(gRegionAllocPtr++, 1, 3, 2, 0, 0);
        gSP1Quadrangle(gRegionAllocPtr++, 2, 3, 1, 0, 0);

        // @recomp End the shadow's matrix group after drawing it.
        popRacePlayerMatrixGroup();
    }
}

RECOMP_PATCH void drawRacePlayerModel(RacePlayer *player) {
    RacePlayer *countPlayer;
    RacePlayer *partMatrixPlayer;
    Transform3D *partSource;
    RacePlayer *drawPlayer;
    s32 i;

    countPlayer = player;
    if (gRenderMatricesDirty != 0) {
        player->stateFlags |= RACE_PLAYER_MODEL_RENDERER_FLAG_MODEL_MATRICES_READY;
        i = 0;
        if (player->modelPartCount > 0) {
            partMatrixPlayer = player;
            partSource = player->modelPartTransforms;
            do {
                partMatrixPlayer->modelPartMatrices[0] = allocFixedTransformMatrix(partSource);
                if (partMatrixPlayer->modelPartMatrices[0] == NULL) {
                    player->stateFlags &= ~RACE_PLAYER_MODEL_RENDERER_FLAG_MODEL_MATRICES_READY;
                }
                i++;
                // Advance one matrix slot while retaining the source's matching induction-variable shape.
                partMatrixPlayer = (RacePlayer *)((Mtx **)partMatrixPlayer + 1);
                partSource++;
            } while (i < countPlayer->modelPartCount);
        }
    }

    drawPlayer = player;
    if ((drawPlayer->stateFlags & RACE_PLAYER_MODEL_RENDERER_FLAG_MODEL_MATRICES_READY) == 0) {
        return;
    }

    // @recomp Give the race player's snowboard a unique ID
    pushRacePlayerBoneMatrixGroup(drawPlayer->playerIndex, RACE_PLAYER_BONE_ID_BOARD);

    drawSnowboardModel(
        drawPlayer->modelPartMatrices[0],
        drawPlayer->characterVariant,
        drawPlayer->snowboardTextureIndex
    );

    // @recomp End the snowboard's matrix group after drawing it.
    popRacePlayerMatrixGroup();

    if (drawPlayer->actionSoundTimer != 0) {
        if (drawPlayer->actionSoundTimer < 0xA5 && drawPlayer->actionSoundTimer >= 0x10) {
            return;
        }
        if ((gUiBlinkTimer & 1) != 0) {
            return;
        }
    }

    if ((drawPlayer->stateFlags & RACE_PLAYER_MODEL_RENDERER_FLAG_HIDE_MESHES) == 0) {
        gDPPipeSync(gRegionAllocPtr++);
        gSPSegment(gRegionAllocPtr++, 2, getRelocatableHeapBlockBase(gAssetHandles[drawPlayer->playerIndex + 0xE]));
        gSPSegment(gRegionAllocPtr++, 3, getRelocatableHeapBlockBase(gAssetHandles[drawPlayer->playerIndex + 0x12]));

        for (i = 0; i < 13; i++) {
            // @recomp Give each body part a unique ID
            pushRacePlayerBoneMatrixGroup(drawPlayer->playerIndex, i + 1);
            gSPMatrix(
                gRegionAllocPtr++,
                drawPlayer->modelPartMatrices[i + 1],
                G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW
            );
            gSPDisplayList(
                gRegionAllocPtr++,
                gRacePlayerModelPartDisplayLists[i][drawPlayer->characterId]
            );
            // @recomp End the unique ID
            popRacePlayerMatrixGroup();
        }
    }
}

RECOMP_PATCH void drawRaceGhostPlayerModel(RacePlayer *player) {
    RacePlayer *countPlayer;
    RacePlayer *partMatrixPlayer;
    Transform3D *partSource;
    s32 i;
    s32 alphaPulse;
    Gfx *segmentGfx;

    countPlayer = player;
    if (gRenderMatricesDirty != 0) {
        player->stateFlags |= RACE_PLAYER_MODEL_RENDERER_FLAG_MODEL_MATRICES_READY;
        i = 0;
        if (player->modelPartCount > 0) {
            partMatrixPlayer = player;
            partSource = player->modelPartTransforms;
            do {
                // Folded away by IDO, but preserves the target's saved-register allocation.
                if ((partMatrixPlayer && partMatrixPlayer) && partMatrixPlayer) {}
                partMatrixPlayer->modelPartMatrices[0] = allocFixedTransformMatrix(partSource);
                if (partMatrixPlayer->modelPartMatrices[0] == NULL) {
                    player->stateFlags &= ~RACE_PLAYER_MODEL_RENDERER_FLAG_MODEL_MATRICES_READY;
                }
                i++;
                // Advance one matrix slot while retaining the source's matching induction-variable shape.
                partMatrixPlayer = (RacePlayer *)((Mtx **)partMatrixPlayer + 1);
                partSource++;
            } while (i < countPlayer->modelPartCount);
        }
    }

    if ((player->stateFlags & RACE_PLAYER_MODEL_RENDERER_FLAG_MODEL_MATRICES_READY) == 0) {
        return;
    }

    alphaPulse = gUiBlinkTimer & 0x1F;
    if (alphaPulse >= 0x10) {
        alphaPulse = 0x1F - alphaPulse;
    }
    alphaPulse = (alphaPulse * 4) + 0x26;
    gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0, 0, 0, alphaPulse);

    // @recomp Give the ghost's snowboard a unique ID
    pushRacePlayerBoneMatrixGroup(player->playerIndex, RACE_PLAYER_BONE_ID_BOARD);
    drawGhostSnowboardModel(player->modelPartMatrices[0], player->characterVariant, player->snowboardTextureIndex);
    // @recomp End the unique ID
    popRacePlayerMatrixGroup();

    if ((player->stateFlags & RACE_PLAYER_MODEL_RENDERER_FLAG_HIDE_MESHES) == 0) {
        gDPPipeSync(gRegionAllocPtr++);
        gSPSegment(gRegionAllocPtr++, 2, getRelocatableHeapBlockBase(gAssetHandles[player->playerIndex + 0xE]));
        segmentGfx = gRegionAllocPtr++;
        gSPSegment(segmentGfx, 3, getRelocatableHeapBlockBase(gAssetHandles[player->playerIndex + 0x12]));
        for (i = 0; i < 13; i++) {
            // @recomp Give each ghost body part a unique ID
            pushRacePlayerBoneMatrixGroup(player->playerIndex, i + 1);
            gSPMatrix(
                gRegionAllocPtr++,
                player->modelPartMatrices[i + 1],
                G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW
            );
            gSPDisplayList(gRegionAllocPtr++, gRaceGhostPlayerModelPartDisplayLists[i][player->characterId]);
            // @recomp End the unique ID
            popRacePlayerMatrixGroup();
        }
    }
}
