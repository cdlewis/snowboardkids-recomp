#include "patches.h"

#include "transform_ids.h"
#include "assets.h"
#include "game/race/course/race_course_effects.h"
#include "game/race/course/race_course_props_and_pickups.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/system_runtime.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"
#include "game/race/camera/race_camera.h"

#define ASSET_HANDLE(index) (gAssetHandles[(index)])
#define THROWN_PICKUP_COURSE_OBJECT_MODEL_INDEX 12

extern Gfx *gRaceCourseObjectDisplayLists[];
extern u8 gRaceUpdatePaused;

static u16 nextFallingRockSpawnId;

RECOMP_PATCH void renderPatrolPenguin(PatrolPenguinActor *arg0) {
    s32 sine;
    s32 doubleSine;
    Transform3D transform;
    volatile s32 pad0[18];

    if (gRenderMatricesDirty != 0) {
        arg0->matrixValid = 0;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->position) != 0) {
        if (arg0->matrixValid == 0) {
            arg0->matrixValid = 1;
            sine = fixedSine(arg0->animationPhase);
            if (1) {
                doubleSine = fixedSine((s16)(arg0->animationPhase * 2));
                sine >>= 4;
                makeFixedRotationY(transform.rotation, arg0->angle + sine + 0x800);
                transform.translation.x = arg0->position.x;
                transform.translation.y = (arg0->position.y + (((doubleSine + 0x1000) << 2) << 2)) + 0xA4000;
            }
            transform.translation.z = arg0->position.z;
            scaleFixedMatrix3sByQuarter(transform.rotation);
            arg0->matrix = allocFixedTransformMatrix(&transform);
        }

        if ((((&transform) && (&transform)) && (&transform)) & 0xFFFFu) {}

        if (arg0->matrix != NULL) {
            gDPPipeSync(gRegionAllocPtr++);
            gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0xA)));
            gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0xB)));
            // @recomp Use the persistent penguin task address and viewport to prevent matches to another penguin.
            gEXMatrixGroupSimple(gRegionAllocPtr++, MODELVIEW_SUNSET_PENGUIN_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_SUNSET_VIEWPORT_SHIFT) |
                                 (((u32)arg0 >> 2) & MODELVIEW_SUNSET_ACTOR_MASK),
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(gRegionAllocPtr++, arg0->matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(gRegionAllocPtr++, &_149610_VRAM);
            // @recomp End this actor's matrix identity before the next course object is drawn.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void renderFallingRock(FallingRockActor *arg0) {
    struct {
        Transform3D transform;
        s16 unused[2];
    } scratch;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            makeFixedRotationXY(scratch.transform.rotation, arg0->pitch, arg0->yaw);
            scratch.transform.translation.x = arg0->pos.x;
            scratch.transform.translation.y = arg0->pos.y + 0x190000;
            scratch.transform.translation.z = arg0->pos.z;
            arg0->matrix = allocFixedTransformMatrix(&scratch.transform);
            arg0->matrixDirty = 0;
        }

        if (arg0->matrix != NULL) {
            gDPPipeSync(gRegionAllocPtr++);
            gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(gAssetHandles[0xA]));
            gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(gAssetHandles[0xB]));
            // @recomp Use the rock spawn ID and viewport so recycled task slots never match the previous rock.
            gEXMatrixGroupSimple(gRegionAllocPtr++, MODELVIEW_SUNSET_FALLING_ROCK_ID_BASE |
                                 ((u32)gCurrentViewportIndex << MODELVIEW_SUNSET_VIEWPORT_SHIFT) |
                                 arg0->task.userId,
                                 G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(gRegionAllocPtr++, arg0->matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(
                gRegionAllocPtr++, gRaceCourseObjectDisplayLists[THROWN_PICKUP_COURSE_OBJECT_MODEL_INDEX]
            );
            // @recomp End this actor's matrix identity before the next course object is drawn.
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
        }
    }
}

RECOMP_PATCH void initFallingRock(FallingRockActor *arg0) {
    // @recomp Preserve the decomp scratch layout; Scratch674B4 is private to its source file.
    struct {
        Transform3D transform;
        s32 pad[3];
    } sp1C;
    FallingRockActor *temp_a3 = arg0;

    if (gRaceUpdatePaused == 0) {
        // @recomp The rock never uses task.userId; assign a new lifetime ID before its first render.
        temp_a3->task.userId = ++nextFallingRockSpawnId;
        makeFixedRotationY(sp1C.transform.rotation, temp_a3->yaw);
        temp_a3->timer = 0x32;
        temp_a3->velocity.x = 0;
        temp_a3->velocity.y = 0xB0000;
        temp_a3->velocity.z = 0xFFF90000;
        transformVec3iByFixedMatrix(sp1C.transform.rotation, &temp_a3->velocity, &temp_a3->transformedPos);
        setCallbackTaskCallback(temp_a3, (CallbackTaskCallback)updateFallingRock);
    }
}
