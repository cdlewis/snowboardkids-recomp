#include "patches.h"
#include "transform_ids.h"
#include "camera_interpolation.h"
#include "game/race/race_state.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/system_runtime.h"
#include "game/race/course/race_course_effects.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/asset_manager.h"
#include "game/math/spatial_math.h"
#include "game/race/camera/race_camera.h"

extern RaceCourseTextureMarkerEntry *gCourseTextureMarkerSpawnEntriesByCourse[];

#define ASSET_HANDLE(index) (gAssetHandles[(index)])
#define RACE_COURSE_EFFECTS_GFX_CMD(pkt, cmd0, cmd1) \
    { \
        Gfx *_g = (Gfx *)(pkt); \
        _g->words.w0 = (cmd0); \
        _g->words.w1 = (cmd1); \
    }

RECOMP_PATCH void renderCourseTextureMarkers(RaceCourseObjectMatrixEffect *arg0) {
    volatile u8 pad[8];
    void *image;
    void *palette;
    s16 width;
    s16 height;
    RaceCourseTextureMarkerEntry *entry;
    s16 textureIndex;
    s32 i;

    textureIndex = -1;
    gSPDisplayList(gRegionAllocPtr++, gEffectRenderModeSetupDl);
    entry = gCourseTextureMarkerSpawnEntriesByCourse[gRaceCourseIndex.signedValue];
    i = 0;
    if (entry->type != -1) {
        do {
            if (isPositionNearCurrentRaceViewportCamera(&entry->position) != 0) {
                if (entry->type != textureIndex) {
                    textureIndex = entry->type;
                    getAssetTableImagePaletteAndSize(
                        (u8 *)getRelocatableHeapBlockBase((s32)ASSET_HANDLE(0x1C)),
                        textureIndex,
                        &image,
                        &palette,
                        &width,
                        &height
                    );
                    gDPLoadTextureBlock_4b(
                        gRegionAllocPtr++,
                        image,
                        G_IM_FMT_CI,
                        width,
                        height,
                        0,
                        G_TX_CLAMP,
                        G_TX_CLAMP,
                        G_TX_NOMASK,
                        G_TX_NOMASK,
                        G_TX_NOLOD,
                        G_TX_NOLOD
                    );
                    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, palette);
                }
                // @recomp Give markers a unique ID
                u32 id = MODELVIEW_COURSE_TEXTURE_MARKER_ID_BASE |
                         ((u32)gCurrentViewportIndex << MODELVIEW_COURSE_TEXTURE_MARKER_VIEWPORT_SHIFT) |
                         ((u32)(u16)gRaceCourseIndex.signedValue << MODELVIEW_COURSE_TEXTURE_MARKER_COURSE_SHIFT) |
                         (u16)i;
                u32 component = viewportCameraSkipsInterpolation() ? G_EX_COMPONENT_SKIP : G_EX_COMPONENT_INTERPOLATE;
                gEXMatrixGroupSimple(gRegionAllocPtr++, id, G_EX_PUSH, G_MTX_MODELVIEW,
                                     component, component, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP,
                                     G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
                                     G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP);
                gSPMatrix(gRegionAllocPtr++, &arg0->matrices[i], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPMatrix(gRegionAllocPtr++, gViewportMatrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
                {
                    Gfx *_g = gRegionAllocPtr++;
                    _g->words.w0 = 0x0400103F;
                    _g->words.w1 = (u32)&gCourseTextureMarkerVertices[entry->type * 4];
                }
                RACE_COURSE_EFFECTS_GFX_CMD(gRegionAllocPtr++, 0xB1060402, 0x60200);
                // @recomp End this marker identity before drawing another object.
                gEXPopMatrixGroup(gRegionAllocPtr++, G_MTX_MODELVIEW);
            }
            entry++;
            i++;
        } while (-1 != entry->type);
    }
    gSPDisplayList(gRegionAllocPtr++, gEffectRenderModeCleanupDl);
}
