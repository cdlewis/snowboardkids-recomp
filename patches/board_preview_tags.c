#include "patches.h"

#include "transform_ids.h"

#include "game/save_data.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/course_select_ui.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/math/fixed_point_math.h"
#include "game/race/player/race_player_model_renderer.h"
#include "game/race/player/race_player_input.h"

extern u8 gCurrentViewportIndex;
extern Gfx *gRegionAllocPtr;

RECOMP_PATCH void drawCourseSelectSnowboardPreviewIn(CourseSelectCoursePreviewActor *arg0) {
    u8 sp2F;
    unsigned char sp2E;
    u8 var_t0;
    s8 temp_v0_2;
    RacePlayer *temp_v0_3;
    Transform3D sp30;
    u8 var_a3;
    u8 var_v1;
    int temp_v0;

    if ((gCourseSelectViewportSyncState != 0) && (gCurrentViewportIndex == 1)) {
        var_t0 = 0;
    } else {
        var_t0 = gCurrentViewportIndex;
    }
    if ((gCourseSelectSlideStates[var_t0] == 0) || (gCourseSelectSlideStates[var_t0] & 1)) {
        temp_v0 = arg0->playerFlags[var_t0];
        if ((temp_v0 == 0) || (temp_v0 & 1)) {
            if (gCourseSelectSlideStates[var_t0] == 1) {
                var_a3 = arg0->playerSlots[var_t0].courseIndex;
            } else {
                var_a3 = gRacePlayers[var_t0].menuSelection;
            }
            temp_v0_2 = gGameSaveDataBuffer[var_t0].courseUnlockStates[var_a3];
            if (temp_v0_2 == -1) {
                var_v1 = 9;
            } else {
                var_v1 = temp_v0_2;
            }
            if ((gCourseSelectViewportSyncState != 0) && (gCurrentViewportIndex == 1)) {
                var_v1 = (u8)(gCourseSelectViewportSyncState - 1);
            }
            temp_v0_3 = &gRacePlayers[gCurrentViewportIndex];
            if (temp_v0_3->selectedCharacterId == 5) {
                var_v1 = 0;
                var_a3 = (var_a3 % 3) + 0xC;
            }
            if (temp_v0_3->menuSelection >= 9) {
                var_v1 = 0;
            }
            sp2E = var_v1;
            sp2F = var_a3;
            composeFixedTransforms(&arg0->viewTransform, &arg0->modelTransforms[var_t0], &sp30);
            arg0->renderMatrix = allocFixedTransformMatrix(&sp30);
            if (arg0->renderMatrix != 0) {
                // @recomp Add a stable ID for the board model
                u32 id = MODELVIEW_BOARD_PREVIEW_ID_BASE |
                         ((u32)gCurrentViewportIndex << MODELVIEW_BOARD_PREVIEW_VIEWPORT_SHIFT) | sp2F;
                gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                                     G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                     G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                     G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
                drawSnowboardModel(arg0->renderMatrix, (s16)sp2F, (s16)sp2E);
                // @recomp End the group so later draws do not inherit this board's ID.
                gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            }
        }
    }
}

RECOMP_PATCH void drawCourseSelectSnowboardPreviewOut(CourseSelectCoursePreviewActor *arg0) {
    u8 sp2F;
    unsigned char sp2E;
    u8 var_t0;
    s8 temp_v0_2;
    RacePlayer *temp_v0_3;
    Transform3D sp30;
    u8 var_a3;
    u8 var_v1;
    int temp_v0;

    if ((gCourseSelectViewportSyncState != 0) && (gCurrentViewportIndex == 1)) {
        var_t0 = 0;
    } else {
        var_t0 = gCurrentViewportIndex;
    }
    if ((gCourseSelectSlideStates[var_t0] == 2) || (gCourseSelectSlideStates[var_t0] & 1)) {
        temp_v0 = arg0->playerFlags[var_t0];
        if ((temp_v0 == 0) || (temp_v0 & 1)) {
            if (gCourseSelectSlideStates[var_t0] == 3) {
                var_a3 = arg0->playerSlots[var_t0].courseIndex;
            } else {
                var_a3 = gRacePlayers[var_t0].menuSelection;
            }
            temp_v0_2 = gGameSaveDataBuffer[var_t0].courseUnlockStates[var_a3];
            if (temp_v0_2 == -1) {
                var_v1 = 9;
            } else {
                var_v1 = temp_v0_2;
            }
            if ((gCourseSelectViewportSyncState != 0) && (gCurrentViewportIndex == 1)) {
                var_v1 = (u8)(gCourseSelectViewportSyncState - 1);
            }
            temp_v0_3 = &gRacePlayers[gCurrentViewportIndex];
            if (temp_v0_3->selectedCharacterId == 5) {
                var_v1 = 0;
                var_a3 = (var_a3 % 3) + 0xC;
            }
            if (temp_v0_3->menuSelection >= 9) {
                var_v1 = 0;
            }
            sp2E = var_v1;
            sp2F = var_a3;
            composeFixedTransforms(&arg0->viewTransform, &arg0->modelTransforms[var_t0], &sp30);
            arg0->renderMatrix = allocFixedTransformMatrix(&sp30);
            if (arg0->renderMatrix != 0) {
                // @recomp Add a stable ID for the board model
                u32 id = MODELVIEW_BOARD_PREVIEW_ID_BASE |
                         ((u32)gCurrentViewportIndex << MODELVIEW_BOARD_PREVIEW_VIEWPORT_SHIFT) |
                         (1U << MODELVIEW_BOARD_PREVIEW_ROLE_SHIFT) | sp2F;
                gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                                     G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                                     G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                                     G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
                drawSnowboardModel(arg0->renderMatrix, (s16)sp2F, (s16)sp2E);
                // @recomp End the group so later draws do not inherit this board's ID.
                gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            }
        }
    }
}
