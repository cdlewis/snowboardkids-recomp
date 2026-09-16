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

extern RaceCourseGateEntry gCourseGateSoundParams[];

RECOMP_PATCH void renderCourseGateObject(RaceCourseGateEffect *arg0) {
    Transform3D scratch;
    volatile s32 pad[1];
    RaceCourseGateEffect *temp_s0 = arg0;

    if (gRenderMatricesDirty != 0) {
        temp_s0->baseMatrix = NULL;
        temp_s0->barMatrix = NULL;
        temp_s0->secondPanelMatrix = NULL;
    }

    if (isPositionNearCurrentRaceViewportCamera(&gCourseGateSoundParams[gRaceCourseIndex.signedValue].position) == 0) {
        return;
    }

    if (temp_s0->baseMatrix == NULL) {
        temp_s0->baseMatrix = allocFixedTransformMatrix(&temp_s0->baseTransform);
    }

    if (temp_s0->baseMatrix != NULL) {
        gDPPipeSync(gRegionAllocPtr++);
        gSPSegment(gRegionAllocPtr++, 2, getRelocatableHeapBlockBase(gAssetHandles[0xA]));
        gSPSegment(gRegionAllocPtr++, 3, getRelocatableHeapBlockBase(gAssetHandles[0xB]));
        gSPMatrix(gRegionAllocPtr++, temp_s0->baseMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(gRegionAllocPtr++, &_148F88_VRAM);
    }

    if (temp_s0->barMatrix == NULL) {
        makeFixedRotationZY(
            scratch.rotation,
            gCourseGateSoundParams[gRaceCourseIndex.signedValue].angle,
            temp_s0->barAngle
        );
        scratch.translation.x = temp_s0->barPosition.x;
        scratch.translation.y = temp_s0->barPosition.y;
        scratch.translation.z = temp_s0->barPosition.z;
        temp_s0->barMatrix = allocFixedTransformMatrix(&scratch);
    }

    if (temp_s0->barMatrix != NULL) {
        // @recomp Give the lift bar a unique ID
        u32 id = MODELVIEW_COURSE_GATE_BAR_ID_BASE |
                 ((u32)gCurrentViewportIndex << MODELVIEW_COURSE_GATE_VIEWPORT_SHIFT) |
                 (u16)gRaceCourseIndex.signedValue;
        gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                             G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                             G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                             G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
        gSPMatrix(gRegionAllocPtr++, temp_s0->barMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(gRegionAllocPtr++, &_149040_VRAM);
        // @recomp End the bar's matrix group
        gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
    }

    if (temp_s0->secondPanelMatrix == NULL) {
        scratch = temp_s0->baseTransform;
        scratch.translation.x = temp_s0->secondPanelPosition.x;
        scratch.translation.y = temp_s0->secondPanelPosition.y;
        scratch.translation.z = temp_s0->secondPanelPosition.z;
        temp_s0->secondPanelMatrix = allocFixedTransformMatrix(&scratch);
    }

    if (temp_s0->secondPanelMatrix != NULL) {
        gSPMatrix(gRegionAllocPtr++, temp_s0->secondPanelMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        if (temp_s0->isOpen == 0) {
            gSPDisplayList(gRegionAllocPtr++, &_149120_VRAM);
        } else {
            gSPDisplayList(gRegionAllocPtr++, &_1491F8_VRAM);
        }
    }
}
