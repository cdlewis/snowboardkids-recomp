#include "patches.h"

#include "transform_ids.h"
#include "assets.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/course/race_course_effects.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/system_runtime.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"
#include "game/race/camera/race_camera.h"

#define RACE_UI_TRAIL_GFX_ALLOC_PTR gRegionAllocPtr
#define ASSET_HANDLE(index) (gAssetHandles[(index)])

RECOMP_PATCH void renderRaceCourseTripleParticle(RaceUiTripleParticleActor *arg0) {
    s16 unused;
    Transform3D spAC;
    Transform3D sp8C;
    Transform3D sp6C;
    s32 sine;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            makeFixedRotationY(spAC.rotation, arg0->rotY);
            spAC.translation.x = arg0->pos.x;
            spAC.translation.y = arg0->pos.y;
            spAC.translation.z = arg0->pos.z;

            sp8C = sp6C = spAC;

            sine = fixedSine((s16)(arg0->rotY << 4)) << 7;
            sp8C.translation.y = (sp8C.translation.y - sine) + 0x80000;
            sp6C.translation.y += sine + 0x80000;

            arg0->matrix0 = allocFixedTransformMatrix(&spAC);
            arg0->matrix1 = allocFixedTransformMatrix(&sp8C);
            arg0->matrix2 = allocFixedTransformMatrix(&sp6C);
        }

        if ((arg0->matrix0 != NULL) && (arg0->matrix1 != NULL) && (arg0->matrix2 != NULL)) {
            gDPPipeSync(RACE_UI_TRAIL_GFX_ALLOC_PTR++);
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
            // @recomp Give ride parts a unique ID
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_TRIPLE_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (0u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->index,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->matrix0, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_TRIPLE_PARTICLE_CENTER_DISPLAY_LIST_VRAM);
            // @recomp End this part identity
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
            // @recomp Give ride parts a unique ID
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_TRIPLE_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (1u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->index,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->matrix1, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_TRIPLE_PARTICLE_NEGATIVE_SINE_OFFSET_DISPLAY_LIST_VRAM);
            // @recomp End this part identity
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
            // @recomp Give ride parts a unique ID
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_TRIPLE_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (2u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->index,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->matrix2, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_TRIPLE_PARTICLE_POSITIVE_SINE_OFFSET_DISPLAY_LIST_VRAM);
            // @recomp End this part identity
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void renderDizzyLandTrailingParticle(RaceUiTrailingParticleActor *arg0) {
    struct {
        Transform3D transform;
        s16 unused[2];
    } scratch;
    Vec3i transformedOffset;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            makeFixedRotationY(scratch.transform.rotation, arg0->rotY);
            scratch.transform.translation.x = arg0->pos.x;
            scratch.transform.translation.y = arg0->pos.y;
            scratch.transform.translation.z = arg0->pos.z;
            arg0->matrix0 = allocFixedTransformMatrix(&scratch.transform);

            transformVec3iByFixedMatrix(scratch.transform.rotation, &gDizzyLandTrailingParticleLocalOffset, &transformedOffset);
            scratch.transform.translation.x += transformedOffset.x;
            scratch.transform.translation.y += transformedOffset.y;
            scratch.transform.translation.z += transformedOffset.z;
            makeFixedRotationZY(scratch.transform.rotation, arg0->rotY, arg0->rotX);
            arg0->matrix1 = allocFixedTransformMatrix(&scratch.transform);
        }

        if (arg0->matrix1 != NULL) {
            gDPPipeSync(RACE_UI_TRAIL_GFX_ALLOC_PTR++);
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
            // @recomp Match each ride part by renderer, viewport, part number and persistent course-spawn index.
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_TRAILING_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (0u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->index,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->matrix0, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_TRAILING_PARTICLE_BASE_DISPLAY_LIST_VRAM);
            // @recomp End this part identity so the next independently moving transform has its own match.
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
            // @recomp Match each ride part by renderer, viewport, part number and persistent course-spawn index.
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_TRAILING_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (1u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->index,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->matrix1, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_TRAILING_PARTICLE_DISPLAY_LIST_VRAM);
            // @recomp End this part identity so the next independently moving transform has its own match.
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void renderRaceCourseSpinningObject(RaceUiSpinningParticleActor *arg0) {
    struct {
        Transform3D transform;
        s16 unused[2];
    } scratch;
    s32 temp2;
    s32 pad;
    s32 pad2;
    SplitWord temp;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) == 0) {
        return;
    }

    if (arg0->matrixDirty != 0) {
        arg0->matrixDirty = 0;
        makeFixedRotationY(scratch.transform.rotation, arg0->rotY);
        scratch.transform.translation.x = arg0->pos.x;
        scratch.transform.translation.y = arg0->pos.y;
        scratch.transform.translation.z = arg0->pos.z;
        arg0->matrix0 = allocFixedTransformMatrix(&scratch.transform);

        scratch.transform.translation.y += 0x01000000;
        temp.half.lo = fixedSine(arg0->rotX) >> 5;
        temp2 = fixedSine(arg0->rotX2) >> 5;
        makeFixedRotationYZX(scratch.transform.rotation, temp.half.lo, arg0->rotZ, temp2);
        arg0->matrix1 = allocFixedTransformMatrix(&scratch.transform);
    }

    if (arg0->matrix1 != NULL) {
        gDPPipeSync(gRegionAllocPtr++);
        gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
        gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
        // @recomp Match each ride part by renderer, viewport, part number and persistent course-spawn index.
        gEXMatrixGroupSimple(gRegionAllocPtr++, MODELVIEW_DIZZY_LAND_SPINNING_ID_BASE |
                             ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                             (0u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->index,
                             G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
        gSPMatrix(gRegionAllocPtr++, arg0->matrix0, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(gRegionAllocPtr++, &DIZZY_LAND_SPINNING_OBJECT_BASE_DISPLAY_LIST_VRAM);
        // @recomp End this part identity so the next independently moving transform has its own match.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
        // @recomp Match each ride part by renderer, viewport, part number and persistent course-spawn index.
        gEXMatrixGroupSimple(gRegionAllocPtr++, MODELVIEW_DIZZY_LAND_SPINNING_ID_BASE |
                             ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                             (1u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->index,
                             G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
        gSPMatrix(gRegionAllocPtr++, arg0->matrix1, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(gRegionAllocPtr++, &DIZZY_LAND_SPINNING_OBJECT_UPPER_DISPLAY_LIST_VRAM);
        // @recomp End this part identity so the next independently moving transform has its own match.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}
