#include "patches.h"

#include "transform_ids.h"
#include "game/race/items/race_item_projectiles.h"
#include "game/race/items/race_item_effects.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/course/race_course_effects.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/system_runtime.h"
#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"
#include "game/race/camera/race_camera.h"

RECOMP_PATCH void renderWideHomingItemProjectile(RaceItemProjectileActor *arg0) {
    RaceEffectMatrixScratch sp6C;
    volatile u8 padding[8];
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
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            sp6C.source = gIdentityFixedTransform;
            sp6C.source.translation.x = arg0->pos.x;
            sp6C.source.translation.y = arg0->pos.y;
            sp6C.source.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&sp6C.source);
        }

        // @recomp Give projectiles a unique ID
        u32 id = MODELVIEW_WIDE_HOMING_ITEM_PROJECTILE_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        do { \
            if (arg0->matrix != NULL) { \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeSetupDl; \
                }; \
                temp_v0_2 = gRegionAllocPtr++; \
                temp_v0_2->words.w0 = 0xFD500000; \
                temp_v0_2->words.w1 = (u32)arg0->image; \
                temp_v0_3 = gRegionAllocPtr++; \
                temp_v0_3->words.w0 = 0xF5500000; \
                temp_v0_3->words.w1 = 0x07080200; \
                temp_v0_4 = gRegionAllocPtr++; \
                temp_v0_4->words.w1 = 0; \
                temp_v0_4->words.w0 = 0xE6000000; \
                temp_v0_5 = gRegionAllocPtr++; \
                temp_v0_5->words.w0 = 0xF3000000; \
                temp_v0_5->words.w1 = 0x0703F800; \
                temp_v0_6 = gRegionAllocPtr++; \
                temp_v0_6->words.w1 = 0; \
                temp_v0_6->words.w0 = 0xE7000000; \
                temp_v0_7 = gRegionAllocPtr++; \
                temp_v0_7->words.w0 = 0xF5400200; \
                temp_v0_7->words.w1 = 0x00080200; \
                temp_v0_8 = gRegionAllocPtr++; \
                temp_v0_8->words.w0 = 0xF2000000; \
                temp_v0_8->words.w1 = 0x0003C03C; \
                temp_v0_9 = gRegionAllocPtr++; \
                temp_v0_9->words.w0 = 0xFD100000; \
                temp_v0_9->words.w1 = (u32)arg0->palette; \
                temp_v0_10 = gRegionAllocPtr++; \
                temp_v0_10->words.w1 = 0; \
                temp_v0_10->words.w0 = 0xE8000000; \
                temp_v0_11 = gRegionAllocPtr++; \
                temp_v0_11->words.w0 = 0xF5000100; \
                temp_v0_11->words.w1 = 0x07000000; \
                temp_v0_12 = gRegionAllocPtr++; \
                temp_v0_12->words.w1 = 0; \
                temp_v0_12->words.w0 = 0xE6000000; \
                temp_v0_13 = gRegionAllocPtr++; \
                temp_v0_13->words.w0 = 0xF0000000; \
                temp_v0_13->words.w1 = 0x0703C000; \
                temp_v0_14 = gRegionAllocPtr++; \
                temp_v0_14->words.w1 = 0; \
                temp_v0_14->words.w0 = 0xE7000000; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x02) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)arg0->matrix; \
                }; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x00) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gViewportMatrix; \
                }; \
                temp_v0_17 = gRegionAllocPtr++; \
                temp_v0_17->words.w0 = 0x0400103F; \
                temp_v0_17->words.w1 = (u32)gRaceItemProjectileQuadVertices; \
                temp_v0_18 = gRegionAllocPtr++; \
                temp_v0_18->words.w0 = 0xB1060402; \
                temp_v0_18->words.w1 = 0x00060200; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeCleanupDl; \
                }; \
            } \
        } while (0);
        // @recomp End this projectile ID
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderCloseRangeHomingItemProjectile(RaceItemProjectileActor *arg0) {
    RaceEffectMatrixScratch sp64;
    volatile u8 padding[8];
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
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            sp64.source = gIdentityFixedTransform;
            sp64.source.translation.x = arg0->pos.x;
            sp64.source.translation.y = arg0->pos.y;
            sp64.source.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&sp64.source);
        }

        // @recomp Give this projectile a unique ID
        u32 id = MODELVIEW_CLOSE_RANGE_HOMING_ITEM_PROJECTILE_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        do { \
            if (arg0->matrix != NULL) { \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeSetupDl; \
                }; \
                temp_v0_2 = gRegionAllocPtr++; \
                temp_v0_2->words.w0 = 0xFD500000; \
                temp_v0_2->words.w1 = (u32)arg0->image; \
                temp_v0_3 = gRegionAllocPtr++; \
                temp_v0_3->words.w0 = 0xF5500000; \
                temp_v0_3->words.w1 = 0x07080200; \
                temp_v0_4 = gRegionAllocPtr++; \
                temp_v0_4->words.w1 = 0; \
                temp_v0_4->words.w0 = 0xE6000000; \
                temp_v0_5 = gRegionAllocPtr++; \
                temp_v0_5->words.w0 = 0xF3000000; \
                temp_v0_5->words.w1 = 0x0703F800; \
                temp_v0_6 = gRegionAllocPtr++; \
                temp_v0_6->words.w1 = 0; \
                temp_v0_6->words.w0 = 0xE7000000; \
                temp_v0_7 = gRegionAllocPtr++; \
                temp_v0_7->words.w0 = 0xF5400200; \
                temp_v0_7->words.w1 = 0x00080200; \
                temp_v0_8 = gRegionAllocPtr++; \
                temp_v0_8->words.w0 = 0xF2000000; \
                temp_v0_8->words.w1 = 0x0003C03C; \
                temp_v0_9 = gRegionAllocPtr++; \
                temp_v0_9->words.w0 = 0xFD100000; \
                temp_v0_9->words.w1 = (u32)arg0->palette; \
                temp_v0_10 = gRegionAllocPtr++; \
                temp_v0_10->words.w1 = 0; \
                temp_v0_10->words.w0 = 0xE8000000; \
                temp_v0_11 = gRegionAllocPtr++; \
                temp_v0_11->words.w0 = 0xF5000100; \
                temp_v0_11->words.w1 = 0x07000000; \
                temp_v0_12 = gRegionAllocPtr++; \
                temp_v0_12->words.w1 = 0; \
                temp_v0_12->words.w0 = 0xE6000000; \
                temp_v0_13 = gRegionAllocPtr++; \
                temp_v0_13->words.w0 = 0xF0000000; \
                temp_v0_13->words.w1 = 0x0703C000; \
                temp_v0_14 = gRegionAllocPtr++; \
                temp_v0_14->words.w1 = 0; \
                temp_v0_14->words.w0 = 0xE7000000; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x02) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)arg0->matrix; \
                }; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x00) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gViewportMatrix; \
                }; \
                temp_v0_17 = gRegionAllocPtr++; \
                temp_v0_17->words.w0 = 0x0400103F; \
                temp_v0_17->words.w1 = (u32)gRaceItemProjectileQuadVertices; \
                temp_v0_18 = gRegionAllocPtr++; \
                temp_v0_18->words.w0 = 0xB1060402; \
                temp_v0_18->words.w1 = 0x00060200; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeCleanupDl; \
                }; \
            } \
        } while (0);
        // @recomp End this projectile ID before another actor is drawn.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderBouncingItemProjectile(RaceItemProjectileActor *arg0) {
    RaceEffectMatrixScratch sp64;
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
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            sp64.source = gIdentityFixedTransform;
            sp64.source.translation.x = arg0->pos.x;
            sp64.source.translation.y = arg0->pos.y;
            sp64.source.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&sp64.source);
        }

        // @recomp Give this projectile a unique ID
        u32 id = MODELVIEW_BOUNCING_ITEM_PROJECTILE_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        do { \
            if (arg0->matrix != NULL) { \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeSetupDl; \
                }; \
                temp_v0_2 = gRegionAllocPtr++; \
                temp_v0_2->words.w0 = 0xFD500000; \
                temp_v0_2->words.w1 = (u32)arg0->image; \
                temp_v0_3 = gRegionAllocPtr++; \
                temp_v0_3->words.w0 = 0xF5500000; \
                temp_v0_3->words.w1 = 0x07080200; \
                temp_v0_4 = gRegionAllocPtr++; \
                temp_v0_4->words.w1 = 0; \
                temp_v0_4->words.w0 = 0xE6000000; \
                temp_v0_5 = gRegionAllocPtr++; \
                temp_v0_5->words.w0 = 0xF3000000; \
                temp_v0_5->words.w1 = 0x0703F800; \
                temp_v0_6 = gRegionAllocPtr++; \
                temp_v0_6->words.w1 = 0; \
                temp_v0_6->words.w0 = 0xE7000000; \
                temp_v0_7 = gRegionAllocPtr++; \
                temp_v0_7->words.w0 = 0xF5400200; \
                temp_v0_7->words.w1 = 0x00080200; \
                temp_v0_8 = gRegionAllocPtr++; \
                temp_v0_8->words.w0 = 0xF2000000; \
                temp_v0_8->words.w1 = 0x0003C03C; \
                temp_v0_9 = gRegionAllocPtr++; \
                temp_v0_9->words.w0 = 0xFD100000; \
                temp_v0_9->words.w1 = (u32)arg0->palette; \
                temp_v0_10 = gRegionAllocPtr++; \
                temp_v0_10->words.w1 = 0; \
                temp_v0_10->words.w0 = 0xE8000000; \
                temp_v0_11 = gRegionAllocPtr++; \
                temp_v0_11->words.w0 = 0xF5000100; \
                temp_v0_11->words.w1 = 0x07000000; \
                temp_v0_12 = gRegionAllocPtr++; \
                temp_v0_12->words.w1 = 0; \
                temp_v0_12->words.w0 = 0xE6000000; \
                temp_v0_13 = gRegionAllocPtr++; \
                temp_v0_13->words.w0 = 0xF0000000; \
                temp_v0_13->words.w1 = 0x0703C000; \
                temp_v0_14 = gRegionAllocPtr++; \
                temp_v0_14->words.w1 = 0; \
                temp_v0_14->words.w0 = 0xE7000000; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x02) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)arg0->matrix; \
                }; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x00) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gViewportMatrix; \
                }; \
                temp_v0_17 = gRegionAllocPtr++; \
                temp_v0_17->words.w0 = 0x0400103F; \
                temp_v0_17->words.w1 = (u32)gRaceItemProjectileQuadVertices; \
                temp_v0_18 = gRegionAllocPtr++; \
                temp_v0_18->words.w0 = 0xB1060402; \
                temp_v0_18->words.w1 = 0x00060200; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeCleanupDl; \
                }; \
            } \
        } while (0);
        // @recomp End this projectile ID before another actor is drawn.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderThrownTrailImpactProjectile(RaceItemProjectileActor *arg0) {
    RaceEffectMatrixScratch sp64;
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
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            sp64.source = gIdentityFixedTransform;
            sp64.source.translation.x = arg0->pos.x;
            sp64.source.translation.y = arg0->pos.y;
            sp64.source.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&sp64.source);
        }

        // @recomp Give this projectile a unique ID
        u32 id = MODELVIEW_THROWN_TRAIL_IMPACT_PROJECTILE_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        do { \
            if (arg0->matrix != NULL) { \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeSetupDl; \
                }; \
                temp_v0_2 = gRegionAllocPtr++; \
                temp_v0_2->words.w0 = 0xFD500000; \
                temp_v0_2->words.w1 = (u32)arg0->image; \
                temp_v0_3 = gRegionAllocPtr++; \
                temp_v0_3->words.w0 = 0xF5500000; \
                temp_v0_3->words.w1 = 0x07080200; \
                temp_v0_4 = gRegionAllocPtr++; \
                temp_v0_4->words.w1 = 0; \
                temp_v0_4->words.w0 = 0xE6000000; \
                temp_v0_5 = gRegionAllocPtr++; \
                temp_v0_5->words.w0 = 0xF3000000; \
                temp_v0_5->words.w1 = 0x0703F800; \
                temp_v0_6 = gRegionAllocPtr++; \
                temp_v0_6->words.w1 = 0; \
                temp_v0_6->words.w0 = 0xE7000000; \
                temp_v0_7 = gRegionAllocPtr++; \
                temp_v0_7->words.w0 = 0xF5400200; \
                temp_v0_7->words.w1 = 0x00080200; \
                temp_v0_8 = gRegionAllocPtr++; \
                temp_v0_8->words.w0 = 0xF2000000; \
                temp_v0_8->words.w1 = 0x0003C03C; \
                temp_v0_9 = gRegionAllocPtr++; \
                temp_v0_9->words.w0 = 0xFD100000; \
                temp_v0_9->words.w1 = (u32)arg0->palette; \
                temp_v0_10 = gRegionAllocPtr++; \
                temp_v0_10->words.w1 = 0; \
                temp_v0_10->words.w0 = 0xE8000000; \
                temp_v0_11 = gRegionAllocPtr++; \
                temp_v0_11->words.w0 = 0xF5000100; \
                temp_v0_11->words.w1 = 0x07000000; \
                temp_v0_12 = gRegionAllocPtr++; \
                temp_v0_12->words.w1 = 0; \
                temp_v0_12->words.w0 = 0xE6000000; \
                temp_v0_13 = gRegionAllocPtr++; \
                temp_v0_13->words.w0 = 0xF0000000; \
                temp_v0_13->words.w1 = 0x0703C000; \
                temp_v0_14 = gRegionAllocPtr++; \
                temp_v0_14->words.w1 = 0; \
                temp_v0_14->words.w0 = 0xE7000000; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x02) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)arg0->matrix; \
                }; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x00) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gViewportMatrix; \
                }; \
                temp_v0_17 = gRegionAllocPtr++; \
                temp_v0_17->words.w0 = 0x0400103F; \
                temp_v0_17->words.w1 = (u32)gRaceItemProjectileQuadVertices; \
                temp_v0_18 = gRegionAllocPtr++; \
                temp_v0_18->words.w0 = 0xB1060402; \
                temp_v0_18->words.w1 = 0x00060200; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeCleanupDl; \
                }; \
            } \
        } while (0);
        // @recomp End this projectile ID before another actor is drawn.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderAreaBlastItemProjectile(RaceItemProjectileActor *arg0) {
    RaceEffectMatrixScratch sp64;
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
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            sp64.source = gIdentityFixedTransform;
            sp64.source.translation.x = arg0->pos.x;
            sp64.source.translation.y = arg0->pos.y;
            sp64.source.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&sp64.source);
        }

        // @recomp Give this projectile a unique ID
        u32 id = MODELVIEW_AREA_BLAST_ITEM_PROJECTILE_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        do { \
            if (arg0->matrix != NULL) { \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeSetupDl; \
                }; \
                temp_v0_2 = gRegionAllocPtr++; \
                temp_v0_2->words.w0 = 0xFD500000; \
                temp_v0_2->words.w1 = (u32)arg0->image; \
                temp_v0_3 = gRegionAllocPtr++; \
                temp_v0_3->words.w0 = 0xF5500000; \
                temp_v0_3->words.w1 = 0x07080200; \
                temp_v0_4 = gRegionAllocPtr++; \
                temp_v0_4->words.w1 = 0; \
                temp_v0_4->words.w0 = 0xE6000000; \
                temp_v0_5 = gRegionAllocPtr++; \
                temp_v0_5->words.w0 = 0xF3000000; \
                temp_v0_5->words.w1 = 0x0703F800; \
                temp_v0_6 = gRegionAllocPtr++; \
                temp_v0_6->words.w1 = 0; \
                temp_v0_6->words.w0 = 0xE7000000; \
                temp_v0_7 = gRegionAllocPtr++; \
                temp_v0_7->words.w0 = 0xF5400200; \
                temp_v0_7->words.w1 = 0x00080200; \
                temp_v0_8 = gRegionAllocPtr++; \
                temp_v0_8->words.w0 = 0xF2000000; \
                temp_v0_8->words.w1 = 0x0003C03C; \
                temp_v0_9 = gRegionAllocPtr++; \
                temp_v0_9->words.w0 = 0xFD100000; \
                temp_v0_9->words.w1 = (u32)arg0->palette; \
                temp_v0_10 = gRegionAllocPtr++; \
                temp_v0_10->words.w1 = 0; \
                temp_v0_10->words.w0 = 0xE8000000; \
                temp_v0_11 = gRegionAllocPtr++; \
                temp_v0_11->words.w0 = 0xF5000100; \
                temp_v0_11->words.w1 = 0x07000000; \
                temp_v0_12 = gRegionAllocPtr++; \
                temp_v0_12->words.w1 = 0; \
                temp_v0_12->words.w0 = 0xE6000000; \
                temp_v0_13 = gRegionAllocPtr++; \
                temp_v0_13->words.w0 = 0xF0000000; \
                temp_v0_13->words.w1 = 0x0703C000; \
                temp_v0_14 = gRegionAllocPtr++; \
                temp_v0_14->words.w1 = 0; \
                temp_v0_14->words.w0 = 0xE7000000; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x02) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)arg0->matrix; \
                }; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x00) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gViewportMatrix; \
                }; \
                temp_v0_17 = gRegionAllocPtr++; \
                temp_v0_17->words.w0 = 0x0400103F; \
                temp_v0_17->words.w1 = (u32)gRaceItemProjectileQuadVertices; \
                temp_v0_18 = gRegionAllocPtr++; \
                temp_v0_18->words.w0 = 0xB1060402; \
                temp_v0_18->words.w1 = 0x00060200; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeCleanupDl; \
                }; \
            } \
        } while (0);
        // @recomp End this projectile ID before another actor is drawn.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderLongRangeHomingItemProjectile(RaceItemProjectileActor *arg0) {
    RaceEffectMatrixScratch sp64;
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
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            sp64.source = gIdentityFixedTransform;
            sp64.source.translation.x = arg0->pos.x;
            sp64.source.translation.y = arg0->pos.y;
            sp64.source.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&sp64.source);
        }

        // @recomp Give this projectile a unique ID
        u32 id = MODELVIEW_LONG_RANGE_HOMING_ITEM_PROJECTILE_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        do { \
            if (arg0->matrix != NULL) { \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeSetupDl; \
                }; \
                temp_v0_2 = gRegionAllocPtr++; \
                temp_v0_2->words.w0 = 0xFD500000; \
                temp_v0_2->words.w1 = (u32)arg0->image; \
                temp_v0_3 = gRegionAllocPtr++; \
                temp_v0_3->words.w0 = 0xF5500000; \
                temp_v0_3->words.w1 = 0x07080200; \
                temp_v0_4 = gRegionAllocPtr++; \
                temp_v0_4->words.w1 = 0; \
                temp_v0_4->words.w0 = 0xE6000000; \
                temp_v0_5 = gRegionAllocPtr++; \
                temp_v0_5->words.w0 = 0xF3000000; \
                temp_v0_5->words.w1 = 0x0703F800; \
                temp_v0_6 = gRegionAllocPtr++; \
                temp_v0_6->words.w1 = 0; \
                temp_v0_6->words.w0 = 0xE7000000; \
                temp_v0_7 = gRegionAllocPtr++; \
                temp_v0_7->words.w0 = 0xF5400200; \
                temp_v0_7->words.w1 = 0x00080200; \
                temp_v0_8 = gRegionAllocPtr++; \
                temp_v0_8->words.w0 = 0xF2000000; \
                temp_v0_8->words.w1 = 0x0003C03C; \
                temp_v0_9 = gRegionAllocPtr++; \
                temp_v0_9->words.w0 = 0xFD100000; \
                temp_v0_9->words.w1 = (u32)arg0->palette; \
                temp_v0_10 = gRegionAllocPtr++; \
                temp_v0_10->words.w1 = 0; \
                temp_v0_10->words.w0 = 0xE8000000; \
                temp_v0_11 = gRegionAllocPtr++; \
                temp_v0_11->words.w0 = 0xF5000100; \
                temp_v0_11->words.w1 = 0x07000000; \
                temp_v0_12 = gRegionAllocPtr++; \
                temp_v0_12->words.w1 = 0; \
                temp_v0_12->words.w0 = 0xE6000000; \
                temp_v0_13 = gRegionAllocPtr++; \
                temp_v0_13->words.w0 = 0xF0000000; \
                temp_v0_13->words.w1 = 0x0703C000; \
                temp_v0_14 = gRegionAllocPtr++; \
                temp_v0_14->words.w1 = 0; \
                temp_v0_14->words.w0 = 0xE7000000; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x02) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)arg0->matrix; \
                }; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x00) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gViewportMatrix; \
                }; \
                temp_v0_17 = gRegionAllocPtr++; \
                temp_v0_17->words.w0 = 0x0400103F; \
                temp_v0_17->words.w1 = (u32)gRaceItemProjectileQuadVertices; \
                temp_v0_18 = gRegionAllocPtr++; \
                temp_v0_18->words.w0 = 0xB1060402; \
                temp_v0_18->words.w1 = 0x00060200; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeCleanupDl; \
                }; \
            } \
        } while (0);
        // @recomp End this projectile ID before another actor is drawn.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderFallingActionProjectile(RaceItemProjectileActor *arg0) {
    RaceEffectMatrixScratch sp64;
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
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixFlags.matrixDirty2 = 1;
    }

    if ((arg0->blinkTimer < 0x1F) && !(gUiBlinkTimer & 1)) {
        return;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixFlags.matrixDirty2 != 0) {
            arg0->matrixFlags.matrixDirty2 = 0;
            sp64.source = gIdentityFixedTransform;
            sp64.source.translation.x = arg0->pos.x;
            sp64.source.translation.y = arg0->pos.y;
            sp64.source.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&sp64.source);
        }

        // @recomp Give this projectile a unique ID
        u32 id = MODELVIEW_FALLING_ACTION_PROJECTILE_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        do { \
            if (arg0->matrix != NULL) { \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeSetupDl; \
                }; \
                temp_v0_2 = gRegionAllocPtr++; \
                temp_v0_2->words.w0 = 0xFD500000; \
                temp_v0_2->words.w1 = (u32)arg0->image; \
                temp_v0_3 = gRegionAllocPtr++; \
                temp_v0_3->words.w0 = 0xF5500000; \
                temp_v0_3->words.w1 = 0x07080200; \
                temp_v0_4 = gRegionAllocPtr++; \
                temp_v0_4->words.w1 = 0; \
                temp_v0_4->words.w0 = 0xE6000000; \
                temp_v0_5 = gRegionAllocPtr++; \
                temp_v0_5->words.w0 = 0xF3000000; \
                temp_v0_5->words.w1 = 0x0703F800; \
                temp_v0_6 = gRegionAllocPtr++; \
                temp_v0_6->words.w1 = 0; \
                temp_v0_6->words.w0 = 0xE7000000; \
                temp_v0_7 = gRegionAllocPtr++; \
                temp_v0_7->words.w0 = 0xF5400200; \
                temp_v0_7->words.w1 = 0x00080200; \
                temp_v0_8 = gRegionAllocPtr++; \
                temp_v0_8->words.w0 = 0xF2000000; \
                temp_v0_8->words.w1 = 0x0003C03C; \
                temp_v0_9 = gRegionAllocPtr++; \
                temp_v0_9->words.w0 = 0xFD100000; \
                temp_v0_9->words.w1 = (u32)arg0->palette; \
                temp_v0_10 = gRegionAllocPtr++; \
                temp_v0_10->words.w1 = 0; \
                temp_v0_10->words.w0 = 0xE8000000; \
                temp_v0_11 = gRegionAllocPtr++; \
                temp_v0_11->words.w0 = 0xF5000100; \
                temp_v0_11->words.w1 = 0x07000000; \
                temp_v0_12 = gRegionAllocPtr++; \
                temp_v0_12->words.w1 = 0; \
                temp_v0_12->words.w0 = 0xE6000000; \
                temp_v0_13 = gRegionAllocPtr++; \
                temp_v0_13->words.w0 = 0xF0000000; \
                temp_v0_13->words.w1 = 0x0703C000; \
                temp_v0_14 = gRegionAllocPtr++; \
                temp_v0_14->words.w1 = 0; \
                temp_v0_14->words.w0 = 0xE7000000; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x02) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)arg0->matrix; \
                }; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x00) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gViewportMatrix; \
                }; \
                temp_v0_17 = gRegionAllocPtr++; \
                temp_v0_17->words.w0 = 0x0400103F; \
                temp_v0_17->words.w1 = (u32)gFallingActionProjectileQuadVertices; \
                temp_v0_18 = gRegionAllocPtr++; \
                temp_v0_18->words.w0 = 0xB1060402; \
                temp_v0_18->words.w1 = 0x00060200; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeCleanupDl; \
                }; \
            } \
        } while (0);
        // @recomp End this projectile ID before another actor is drawn.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderShieldProjectile(RaceItemProjectileActor *arg0) {
    RaceEffectMatrixScratch sp64;
    volatile u8 padding[8];
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
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty2 = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty2 != 0) {
            arg0->matrixDirty2 = 0;
            sp64.source = gIdentityFixedTransform;
            sp64.source.translation.x = arg0->pos.x;
            sp64.source.translation.y = arg0->pos.y;
            sp64.source.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&sp64.source);
        }

        // @recomp Give this projectile a unique ID
        u32 id = MODELVIEW_SHIELD_PROJECTILE_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        do { \
            if (arg0->matrix != NULL) { \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeSetupDl; \
                }; \
                temp_v0_2 = gRegionAllocPtr++; \
                temp_v0_2->words.w0 = 0xFD500000; \
                temp_v0_2->words.w1 = (u32)arg0->image; \
                temp_v0_3 = gRegionAllocPtr++; \
                temp_v0_3->words.w0 = 0xF5500000; \
                temp_v0_3->words.w1 = 0x07080200; \
                temp_v0_4 = gRegionAllocPtr++; \
                temp_v0_4->words.w1 = 0; \
                temp_v0_4->words.w0 = 0xE6000000; \
                temp_v0_5 = gRegionAllocPtr++; \
                temp_v0_5->words.w0 = 0xF3000000; \
                temp_v0_5->words.w1 = 0x0703F800; \
                temp_v0_6 = gRegionAllocPtr++; \
                temp_v0_6->words.w1 = 0; \
                temp_v0_6->words.w0 = 0xE7000000; \
                temp_v0_7 = gRegionAllocPtr++; \
                temp_v0_7->words.w0 = 0xF5400200; \
                temp_v0_7->words.w1 = 0x00080200; \
                temp_v0_8 = gRegionAllocPtr++; \
                temp_v0_8->words.w0 = 0xF2000000; \
                temp_v0_8->words.w1 = 0x0003C03C; \
                temp_v0_9 = gRegionAllocPtr++; \
                temp_v0_9->words.w0 = 0xFD100000; \
                temp_v0_9->words.w1 = (u32)arg0->palette; \
                temp_v0_10 = gRegionAllocPtr++; \
                temp_v0_10->words.w1 = 0; \
                temp_v0_10->words.w0 = 0xE8000000; \
                temp_v0_11 = gRegionAllocPtr++; \
                temp_v0_11->words.w0 = 0xF5000100; \
                temp_v0_11->words.w1 = 0x07000000; \
                temp_v0_12 = gRegionAllocPtr++; \
                temp_v0_12->words.w1 = 0; \
                temp_v0_12->words.w0 = 0xE6000000; \
                temp_v0_13 = gRegionAllocPtr++; \
                temp_v0_13->words.w0 = 0xF0000000; \
                temp_v0_13->words.w1 = 0x0703C000; \
                temp_v0_14 = gRegionAllocPtr++; \
                temp_v0_14->words.w1 = 0; \
                temp_v0_14->words.w0 = 0xE7000000; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x02) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)arg0->matrix; \
                }; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)1) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)((0x00 | 0x00) | 0x00)) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)(sizeof(Mtx))) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gViewportMatrix; \
                }; \
                temp_v0_17 = gRegionAllocPtr++; \
                temp_v0_17->words.w0 = 0x0400103F; \
                temp_v0_17->words.w1 = (u32)gRaceItemProjectileQuadVertices; \
                temp_v0_18 = gRegionAllocPtr++; \
                temp_v0_18->words.w0 = 0xB1060402; \
                temp_v0_18->words.w1 = 0x00060200; \
                { \
                    Gfx *_g = (Gfx *)(gRegionAllocPtr++); \
                    _g->words.w0 = (((u32)((((u32)6) & ((0x01 << 8) - 1)) << 24)) | \
                                    ((u32)((((u32)0x00) & ((0x01 << 8) - 1)) << 16))) | \
                                   ((u32)((((u32)0) & ((0x01 << 16) - 1)) << 0)); \
                    _g->words.w1 = (u32)gEffectRenderModeCleanupDl; \
                }; \
            } \
        } while (0);
        // @recomp End this projectile ID before another actor is drawn.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderRaceUiProjectile(RaceUiProjectileActor *arg0) {
    volatile s32 pad0;
    Transform3D sp7C;
    volatile u8 padding[4];
    s32 sp74;
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
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;
    Gfx *temp_t4;
    s32 temp_t0;
    s32 var_ra;
    s32 var_t5;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = (float) 1;
    }

    if (arg0->matrixDirty != 0) {
        arg0->matrixDirty = 0;
        sp7C = gIdentityFixedTransform;
        sp7C.translation.x = arg0->pos.x;
        sp7C.translation.y = arg0->pos.y;
        sp7C.translation.z = arg0->pos.z;
        arg0->matrix = allocFixedTransformMatrix(&sp7C);
    }

    if (arg0->matrix != NULL) {
        var_ra = 0x20;
        if (arg0->flags & 4) {
            var_ra = 0x40;
            sp74 = 4;
        } else {
            sp74 = 0;
            do { } while (0);
        }
        // @recomp Give this projectile a unique ID
        u32 id = MODELVIEW_RACE_UI_PROJECTILE_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        do { \
            temp_v0_2 = gRegionAllocPtr++; \
            temp_v0_2->words.w0 = 0x06000000; \
            temp_v0_2->words.w1 = (u32)gAlphaSpriteRenderModeDl; \
            temp_v0_3 = gRegionAllocPtr++; \
            temp_v0_3->words.w0 = 0xFD500000; \
            temp_v0_3->words.w1 = (u32)arg0->image; \
            temp_v0_4 = gRegionAllocPtr++; \
            temp_v0_4->words.w0 = 0xF5500000; \
            temp_v0_4->words.w1 = 0x07080200; \
            temp_v0_5 = gRegionAllocPtr++; \
            temp_v0_5->words.w0 = 0xE6000000; \
            temp_v0_5->words.w1 = 0; \
            temp_t4 = gRegionAllocPtr++; \
            temp_t4->words.w0 = 0xF3000000; \
            temp_t0 = (((var_ra << 5) + 3) >> 2) - 1; \
            if (temp_t0 < 0x7FF) { \
                var_t5 = temp_t0; \
            } else { \
                var_t5 = 0x7FF; \
            } \
            temp_t4->words.w1 = (((var_t5 & 0xFFF) << 0xC) | 0x07000000) | 0x400; \
            temp_v0_6 = gRegionAllocPtr++; \
            temp_v0_6->words.w1 = 0; \
            temp_v0_6->words.w0 = 0xE7000000; \
            temp_v0_7 = gRegionAllocPtr++; \
            temp_v0_7->words.w0 = 0xF5400400; \
            temp_v0_7->words.w1 = 0x00080200; \
            temp_v0_8 = gRegionAllocPtr++; \
            temp_v0_8->words.w0 = 0xF2000000; \
            temp_v0_8->words.w1 = (((var_ra - 1) << 2) & 0xFFF) | 0x0007C000; \
            temp_v0_9 = gRegionAllocPtr++; \
            temp_v0_9->words.w0 = 0xFD100000; \
            temp_v0_9->words.w1 = (u32)arg0->palette; \
            temp_v0_10 = gRegionAllocPtr++; \
            temp_v0_10->words.w1 = 0; \
            temp_v0_10->words.w0 = 0xE8000000; \
            temp_v0_11 = gRegionAllocPtr++; \
            temp_v0_11->words.w0 = 0xF5000100; \
            temp_v0_11->words.w1 = 0x07000000; \
            temp_v0_12 = gRegionAllocPtr++; \
            temp_v0_12->words.w1 = 0; \
            temp_v0_12->words.w0 = 0xE6000000; \
            temp_v0_13 = gRegionAllocPtr++; \
            temp_v0_13->words.w0 = 0xF0000000; \
            temp_v0_13->words.w1 = 0x0703C000; \
            temp_v0_14 = gRegionAllocPtr++; \
            temp_v0_14->words.w1 = 0; \
            temp_v0_14->words.w0 = 0xE7000000; \
            temp_v0_17 = gRegionAllocPtr++; \
            temp_v0_17->words.w0 = 0x01020040; \
            temp_v0_17->words.w1 = (u32)arg0->matrix; \
            temp_v0_18 = gRegionAllocPtr++; \
            temp_v0_18->words.w0 = 0x01000040; \
            temp_v0_18->words.w1 = (u32)gViewportMatrix; \
            temp_v0_2 = gRegionAllocPtr++; \
            temp_v0_2->words.w0 = 0x0400103F; \
            temp_v0_2->words.w1 = (u32)(&D_800D64A0[sp74]); \
            temp_v0_3 = gRegionAllocPtr++; \
            temp_v0_3->words.w0 = 0xB1060402; \
            temp_v0_3->words.w1 = 0x00060200; \
        } while (0);
        // @recomp End this projectile ID before another actor is drawn.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

RECOMP_PATCH void renderRaceItemProjectileTrailEffect(RaceItemEffectActor *arg0) {
    volatile u8 padding[4];
    Transform3D sp64;
    Gfx *temp_v0_2;

    if (gRenderMatricesDirty != 0) {
        arg0->unk34.shorts.state.bytes.matrixDirty = 1;
    }

    if ((u8)arg0->unk34.shorts.state.bytes.matrixDirty != 0) {
        arg0->unk34.shorts.state.bytes.matrixDirty = 0;
        sp64 = gIdentityFixedTransform;
        sp64.rotation[MTX_XX] = arg0->unk30.screen.y << 8;
        sp64.rotation[MTX_YY] = arg0->unk30.screen.y << 8;
        sp64.rotation[MTX_ZZ] = arg0->unk30.screen.y << 8;
        sp64.translation.x = arg0->payload.vec.x;
        sp64.translation.y = arg0->payload.vec.y;
        sp64.translation.z = arg0->payload.vec.z;
        arg0->vector24.fields.word24.velocityX = (s32)allocFixedTransformMatrix(&sp64);
    }

    if (arg0->vector24.fields.word24.velocityX != 0) {
        // @recomp Give this projectile a unique ID
        u32 id = MODELVIEW_RACE_ITEM_PROJECTILE_TRAIL_EFFECT_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_PROJECTILE_VIEWPORT_SHIFT) |
                 (((u32)arg0 >> 2) & MODELVIEW_PROJECTILE_ACTOR_MASK);
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
        gSPDisplayList(gRegionAllocPtr++, gRaceItemEffectTranslucentRenderSetupDl);
        temp_v0_2 = gRegionAllocPtr++;
        temp_v0_2->words.w0 = 0xFA000000;
        temp_v0_2->words.w1 = (arg0->unk30.screen.x & 0xFF) | ~0xFF;
        gDPLoadTextureBlock_4b(
            gRegionAllocPtr++,
            arg0->vector24.fields.word2C.image,
            G_IM_FMT_CI,
            16,
            16,
            0,
            G_TX_CLAMP,
            G_TX_CLAMP,
            0,
            0,
            0,
            0
        );
        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->vector24.fields.word28.palette);
        gSPMatrix(
            gRegionAllocPtr++,
            (Mtx *)arg0->vector24.fields.word24.velocityX,
            G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW
        );
        gSPMatrix(gRegionAllocPtr++, gViewportMatrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
        gSPVertex(gRegionAllocPtr++, (Vtx *)gRaceItemProjectileQuadVertices, 4, 0);
        gSP1Quadrangle(gRegionAllocPtr++, 2, 1, 0, 3, 3);
        // @recomp End this projectile ID before another actor is drawn.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}
