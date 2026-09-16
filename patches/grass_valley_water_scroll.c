#include "patches.h"

#include "common.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/system_runtime.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/math/fixed_point_math.h"
#include "game/math/spatial_math.h"
#include "game/race/course/race_course_effects.h"
#include "game/race/race_state.h"

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

RECOMP_PATCH void renderCourseWaterLayer(RaceCourseWaterLayerActor *arg0) {
    Gfx *segmentGfx;
    Gfx *gfx;
    s32 i;
    s16 vertexCount;
    s16 tileScrollOffset;
    Gfx *newGfx;
    u8 pad[0x10];

    if (gRenderMatricesDirty != 0) {
        arg0->scrolledVertices = allocMenuRenderScratch((arg0->vertexCount * sizeof(Vtx)) + ((u32)pad & 0));
        if (arg0->scrolledVertices != NULL) {
            i = 0;
            if (arg0->vertexCount > 0) {
                do {
                    arg0->scrolledVertices[i] = arg0->sourceVertices[i];
                    // @recomp Keep the water texture coordinates fixed so the tile scroll below moves smoothly
                    i++;
                } while (i < arg0->vertexCount);
            }
        }
    }

    if (arg0->scrolledVertices != NULL) {
        gDPPipeSync(gRegionAllocPtr++);
        newGfx = gRegionAllocPtr++;
        segmentGfx = newGfx;
        gSPSegment(segmentGfx, 2, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
        gSPMatrix(gRegionAllocPtr++, &gIdentityMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(gRegionAllocPtr++, arg0->renderSetupDisplayListAddress);
        gDPLoadTextureBlock_4b(
            gRegionAllocPtr++,
            arg0->texture,
            G_IM_FMT_CI,
            32,
            64,
            0,
            G_TX_WRAP,
            G_TX_WRAP,
            5,
            6,
            0,
            0
        );
        // @recomp Scroll the 32x64 water tile instead of rewriting vertex UVs so RT64 can interpolate the tile offset.
        tileScrollOffset = ((-arg0->textureScrollT) & 0x7FF) >> 5;
        gfx = gRegionAllocPtr++;
        gDPSetTileSize(gfx, G_TX_RENDERTILE, 0, tileScrollOffset << 2, 0, (tileScrollOffset + 64) << 2);
        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->palette);
        gfx = gRegionAllocPtr++;
        vertexCount = arg0->vertexCount;
        gSPVertex(gfx, arg0->scrolledVertices, vertexCount, 0);
        gSPDisplayList(gRegionAllocPtr++, arg0->geometryDisplayListAddress);
    }
}
