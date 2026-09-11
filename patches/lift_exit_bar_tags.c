#include "patches.h"

#include "transform_ids.h"

#include "assets.h"
#include "game/race/race_state.h"
#include "game/race/course/race_course_effects.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/system_runtime.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"
#include "game/race/camera/race_camera.h"

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

RECOMP_PATCH void renderLiftExitCourseObject(RaceMovingCourseObjectEffect *arg0) {
    volatile s32 unused;
    Transform3D transform;
    volatile s32 pad[2];

    if (gRenderMatricesDirty != 0) {
        makeFixedRotationY(transform.rotation, arg0->yaw);
        transform.translation.x = arg0->position.x;
        transform.translation.y = arg0->position.y;
        transform.translation.z = arg0->position.z;
        arg0->matrix = allocFixedTransformMatrix(&transform);
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->position) != 0) {
        if (arg0->matrix != NULL) {
            gDPPipeSync(gRegionAllocPtr++);
            gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0xA)));
            gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0xB)));
            // @recomp Create a unique ID for the lift exit bar
            u32 id = MODELVIEW_LIFT_EXIT_BAR_ID_BASE |
                     ((u32)gCurrentViewportIndex << MODELVIEW_LIFT_EXIT_BAR_VIEWPORT_SHIFT) |
                     (((u32)arg0 >> 2) & MODELVIEW_LIFT_EXIT_BAR_ACTOR_MASK);
            gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                                 G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_SKIP,
                                 G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                 G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
            gSPMatrix(gRegionAllocPtr++, arg0->matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            if (gRaceCourseIndex.signedValue != 8) {
                gSPDisplayList(gRegionAllocPtr++, &_148220_VRAM);
            } else {
                gSPDisplayList(gRegionAllocPtr++, &_14AB28_VRAM);
            }
            // @recomp End the exit bar ID
            gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
        }
    }
}
