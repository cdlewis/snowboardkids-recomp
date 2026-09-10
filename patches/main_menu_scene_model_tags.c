#include "patches.h"

#include "transform_ids.h"

#include "game/engine/relocatable_heap.h"
#include "game/engine/asset_manager.h"
#include "game/menu/main_menu/main_menu_scene_model.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/player/race_player_model_renderer.h"

extern u8 gCurrentViewportIndex;
extern Gfx *gRegionAllocPtr;
extern Gfx *gMainMenuSceneModelPartDisplayLists[];

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

static void pushMainMenuBoneMatrixGroup(MainMenuSceneModel *model, s32 boneIndex) {
    u32 id = MODELVIEW_MAIN_MENU_BONE_ID_BASE |
             ((u32)(gCurrentViewportIndex & MODELVIEW_MAIN_MENU_VIEWPORT_MASK)
              << MODELVIEW_MAIN_MENU_VIEWPORT_SHIFT) |
             ((u32)(model->sceneModelIndex & MODELVIEW_MAIN_MENU_INDEX_MASK) << MODELVIEW_MAIN_MENU_INDEX_SHIFT) |
             ((u32)boneIndex & MODELVIEW_MAIN_MENU_BONE_MASK);

    gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                         G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP,
                         G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_EDIT_NONE,
                         G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO);
}

RECOMP_PATCH void drawMainMenuSceneModel(MainMenuSceneModel *arg0) {
    Transform3D *transform;
    Mtx *matrix;
    s32 partIndex;
    s32 displayListCount;

    if ((u16)arg0->viewportIndex == gCurrentViewportIndex) {
        gDPPipeSync(gRegionAllocPtr++);
        gSPSegment(gRegionAllocPtr++, 0x02,
                   getRelocatableHeapBlockBase(
                       ASSET_HANDLE(MAIN_MENU_SCENE_MODEL_GEOMETRY_HANDLE_BASE +
                                    (u16)arg0->sceneModelIndex)));
        gSPSegment(gRegionAllocPtr++, 0x03,
                   getRelocatableHeapBlockBase(
                       ASSET_HANDLE(MAIN_MENU_SCENE_MODEL_TEXTURE_HANDLE_BASE +
                                    (u16)arg0->sceneModelIndex)));

        displayListCount = MAIN_MENU_SCENE_MODEL_PART_COUNT - 1;
        partIndex = 1; transform = &arg0->partTransforms[1]; do {
            matrix = allocFixedTransformMatrix(transform);
            if (matrix != NULL) {
                // @recomp Give each body part a unique ID
                pushMainMenuBoneMatrixGroup(arg0, partIndex);
                gSPMatrix(gRegionAllocPtr++, matrix,
                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(
                    gRegionAllocPtr++,
                    gMainMenuSceneModelPartDisplayLists[
                        ((u16)arg0->characterIndex * displayListCount) +
                        partIndex - 1]);
                // @recomp End the body part ID
                gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            }
            partIndex++;
            transform++;
        } while (partIndex != MAIN_MENU_SCENE_MODEL_PART_COUNT);

        /* Keep arg0 live through the loop exit for the original register allocation. */
        if (arg0 == NULL) {
        }
    }
}

RECOMP_PATCH void drawTexturedMainMenuSceneModel(MainMenuSceneModel *arg0) {
    MainMenuSceneModel *model;
    Gfx **displayLists;
    Mtx *matrix;
    s32 i;
    s32 stride;

    do {
        if ((u16)arg0->viewportIndex == gCurrentViewportIndex) {
            matrix = allocFixedTransformMatrix(arg0->partTransforms);
            model = arg0;
            if (matrix != NULL) {
                pushMainMenuBoneMatrixGroup(model, 0);
                drawSnowboardModel(matrix, model->snowboardDisplayListIndex, model->snowboardTextureIndex);
                gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            }

            gDPPipeSync(gRegionAllocPtr++);
            gSPSegment(
                gRegionAllocPtr++,
                0x02,
                getRelocatableHeapBlockBase(
                    gAssetHandles[MAIN_MENU_SCENE_MODEL_GEOMETRY_HANDLE_BASE + (u16)model->sceneModelIndex]
                )
            );
            gSPSegment(
                gRegionAllocPtr++,
                0x03,
                getRelocatableHeapBlockBase(
                    gAssetHandles[MAIN_MENU_SCENE_MODEL_TEXTURE_HANDLE_BASE + (u16)model->sceneModelIndex]
                )
            );

            stride = MAIN_MENU_SCENE_MODEL_PART_COUNT - 1;
            displayLists = gMainMenuSceneModelPartDisplayLists;
            for (i = 1; i < MAIN_MENU_SCENE_MODEL_PART_COUNT; i++) {
                matrix = allocFixedTransformMatrix(&model->partTransforms[i]);
                if (matrix != NULL) {
                    pushMainMenuBoneMatrixGroup(model, i);
                    gSPMatrix(gRegionAllocPtr++, matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gSPDisplayList(gRegionAllocPtr++, displayLists[((u16)model->characterIndex * stride) + i - 1]);
                    gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
                }
            }
        }
    } while (0);
}
