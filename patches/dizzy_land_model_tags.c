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

RECOMP_PATCH void renderDizzyLandCarousel(DizzyLandCarouselActor *arg0) {
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
            makeFixedRotationY(spAC.rotation, arg0->yaw);
            spAC.translation.x = arg0->pos.x;
            spAC.translation.y = arg0->pos.y;
            spAC.translation.z = arg0->pos.z;

            sp8C = sp6C = spAC;

            sine = fixedSine((s16)(arg0->yaw << 4)) << 7;
            sp8C.translation.y = (sp8C.translation.y - sine) + 0x80000;
            sp6C.translation.y += sine + 0x80000;

            arg0->canopyMatrix = allocFixedTransformMatrix(&spAC);
            arg0->negativeSineHorsesMatrix = allocFixedTransformMatrix(&sp8C);
            arg0->positiveSineHorsesMatrix = allocFixedTransformMatrix(&sp6C);
        }

        if ((arg0->canopyMatrix != NULL) && (arg0->negativeSineHorsesMatrix != NULL) &&
            (arg0->positiveSineHorsesMatrix != NULL)) {
            gDPPipeSync(RACE_UI_TRAIL_GFX_ALLOC_PTR++);
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
            // @recomp Give ride parts a unique ID
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_CAROUSEL_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (0u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->task.userId,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->canopyMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_CAROUSEL_CANOPY_DISPLAY_LIST_VRAM);
            // @recomp End this part identity
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
            // @recomp Give ride parts a unique ID
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_CAROUSEL_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (1u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->task.userId,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->negativeSineHorsesMatrix,
                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_CAROUSEL_HORSES_NEGATIVE_SINE_DISPLAY_LIST_VRAM);
            // @recomp End this part identity
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
            // @recomp Give ride parts a unique ID
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_CAROUSEL_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (2u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->task.userId,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->positiveSineHorsesMatrix,
                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_CAROUSEL_HORSES_POSITIVE_SINE_DISPLAY_LIST_VRAM);
            // @recomp End this part identity
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void renderDizzyLandFerrisWheel(DizzyLandFerrisWheelActor *arg0) {
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
            makeFixedRotationY(scratch.transform.rotation, arg0->yaw);
            scratch.transform.translation.x = arg0->pos.x;
            scratch.transform.translation.y = arg0->pos.y;
            scratch.transform.translation.z = arg0->pos.z;
            arg0->supportMatrix = allocFixedTransformMatrix(&scratch.transform);

            transformVec3iByFixedMatrix(scratch.transform.rotation, &gDizzyLandFerrisWheelHubOffset, &transformedOffset);
            scratch.transform.translation.x += transformedOffset.x;
            scratch.transform.translation.y += transformedOffset.y;
            scratch.transform.translation.z += transformedOffset.z;
            makeFixedRotationZY(scratch.transform.rotation, arg0->yaw, arg0->wheelAngle);
            arg0->wheelMatrix = allocFixedTransformMatrix(&scratch.transform);
        }

        if (arg0->wheelMatrix != NULL) {
            gDPPipeSync(RACE_UI_TRAIL_GFX_ALLOC_PTR++);
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
            // @recomp Match each ride part by renderer, viewport, part number and persistent course-spawn index.
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_FERRIS_WHEEL_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (0u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->task.userId,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->supportMatrix,
                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_FERRIS_WHEEL_SUPPORT_DISPLAY_LIST_VRAM);
            // @recomp End this part identity so the next independently moving transform has its own match.
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
            // @recomp Match each ride part by renderer, viewport, part number and persistent course-spawn index.
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_FERRIS_WHEEL_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                                 (1u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->task.userId,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->wheelMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_FERRIS_WHEEL_DISPLAY_LIST_VRAM);
            // @recomp End this part identity so the next independently moving transform has its own match.
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void renderDizzyLandSpinningCabinRide(DizzyLandSpinningCabinRideActor *arg0) {
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
        makeFixedRotationY(scratch.transform.rotation, arg0->baseYaw);
        scratch.transform.translation.x = arg0->pos.x;
        scratch.transform.translation.y = arg0->pos.y;
        scratch.transform.translation.z = arg0->pos.z;
        arg0->poleMatrix = allocFixedTransformMatrix(&scratch.transform);

        scratch.transform.translation.y += 0x01000000;
        temp.half.lo = fixedSine(arg0->pitchPhase) >> 5;
        temp2 = fixedSine(arg0->rollPhase) >> 5;
        makeFixedRotationYZX(scratch.transform.rotation, temp.half.lo, arg0->spinAngle, temp2);
        arg0->canopyMatrix = allocFixedTransformMatrix(&scratch.transform);
    }

    if (arg0->canopyMatrix != NULL) {
        gDPPipeSync(gRegionAllocPtr++);
        gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
        gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
        // @recomp Match each ride part by renderer, viewport, part number and persistent course-spawn index.
        gEXMatrixGroupSimple(gRegionAllocPtr++, MODELVIEW_DIZZY_LAND_SPINNING_CABIN_RIDE_ID_BASE |
                             ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                             (0u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->task.userId,
                             G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
        gSPMatrix(gRegionAllocPtr++, arg0->poleMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(gRegionAllocPtr++, &DIZZY_LAND_SPINNING_CABIN_RIDE_POLE_DISPLAY_LIST_VRAM);
        // @recomp End this part identity so the next independently moving transform has its own match.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
        // @recomp Match each ride part by renderer, viewport, part number and persistent course-spawn index.
        gEXMatrixGroupSimple(gRegionAllocPtr++, MODELVIEW_DIZZY_LAND_SPINNING_CABIN_RIDE_ID_BASE |
                             ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) |
                             (1u << MODELVIEW_DIZZY_LAND_PART_SHIFT) | arg0->task.userId,
                             G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
        gSPMatrix(gRegionAllocPtr++, arg0->canopyMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(gRegionAllocPtr++, &DIZZY_LAND_SPINNING_CABIN_RIDE_CANOPY_DISPLAY_LIST_VRAM);
        // @recomp End this part identity so the next independently moving transform has its own match.
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }
}

// @recomp Preserve the decomp scale expression used by the cup collision animation.
#define SCALE_MATRIX_COMPONENT(value, scale) ((value * scale) / 0x1000)

RECOMP_PATCH void renderDizzyLandTeacupBumper(DizzyLandTeacupBumperActor *arg0) {
    struct {
        Transform3D transform;
        s16 unused[2];
    } scratch;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            makeFixedRotationY(scratch.transform.rotation, arg0->yaw);
            scratch.transform.rotation[0] = SCALE_MATRIX_COMPONENT(scratch.transform.rotation[0], arg0->xzScale);
            scratch.transform.rotation[3] = SCALE_MATRIX_COMPONENT(scratch.transform.rotation[3], arg0->xzScale);
            scratch.transform.rotation[6] = SCALE_MATRIX_COMPONENT(scratch.transform.rotation[6], arg0->xzScale);
            scratch.transform.rotation[2] = SCALE_MATRIX_COMPONENT(scratch.transform.rotation[2], arg0->xzScale);
            scratch.transform.rotation[5] = SCALE_MATRIX_COMPONENT(scratch.transform.rotation[5], arg0->xzScale);
            scratch.transform.rotation[8] = SCALE_MATRIX_COMPONENT(scratch.transform.rotation[8], arg0->xzScale);
            scratch.transform.translation.x = arg0->pos.x;
            scratch.transform.translation.y = arg0->pos.y;
            scratch.transform.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&scratch.transform);
        }

        if (arg0->matrix != NULL) {
            gDPPipeSync(RACE_UI_TRAIL_GFX_ALLOC_PTR++);
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
            // @recomp Match each cup by viewport and persistent spawn index 0-9, not changing draw order.
            // @recomp Use naive matrix interpolation so intermediate cup rotations advance instead of reversing.
            // @recomp The same 3x3 matrix interpolation preserves the collision squash in X/Z.
            gEXMatrixGroupSimple(RACE_UI_TRAIL_GFX_ALLOC_PTR++, MODELVIEW_DIZZY_LAND_TEACUP_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_DIZZY_LAND_VIEWPORT_SHIFT) | arg0->task.userId,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_TEACUP_BUMPER_DISPLAY_LIST_VRAM);
            // @recomp End the cup identity before drawing another course object.
            gEXPopMatrixGroup(RACE_UI_TRAIL_GFX_ALLOC_PTR++, G_MTX_MODELVIEW);
        }
    }
}
