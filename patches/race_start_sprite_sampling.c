#include "patches.h"

#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/ui/race_ui_effects.h"

extern Gfx gAlphaSpriteRenderModeDl[];
extern Vtx D_800D69A8[];

RECOMP_PATCH void renderRaceStartOverlay(RaceUiOverlayActor *arg0) {
    volatile u8 pad[0xC];
    Transform3D transform;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }
    if (arg0->matrixDirty != 0) {
        arg0->matrixDirty = 0;
        transform = gIdentityFixedTransform;
        transform.translation.x = arg0->pos.x;
        transform.translation.y = arg0->pos.y;
        transform.translation.z = arg0->pos.z;
        arg0->matrix = allocFixedTransformMatrix(&transform);
    }
    if (arg0->matrix != NULL) {
        gSPDisplayList(gRegionAllocPtr++, gAlphaSpriteRenderModeDl);
        // @recomp Exclude the spare texel row/column
        gDPLoadTextureTile_4b(gRegionAllocPtr++, arg0->panelAImage, G_IM_FMT_CI,
            48, 40, 0, 0, 47, 39, 0, G_TX_CLAMP, G_TX_CLAMP,
            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->panelAPalette);
        gSPMatrix(gRegionAllocPtr++, arg0->matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPMatrix(gRegionAllocPtr++, gViewportMatrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
        gSPVertex(gRegionAllocPtr++, D_800D69A8, 8, 0);
        gSP1Quadrangle(gRegionAllocPtr++, 3, 2, 1, 0, 0);
        // @recomp Exclude the spare texel row/column
        gDPLoadTextureTile_4b(gRegionAllocPtr++, arg0->panelBImage, G_IM_FMT_CI,
            48, 40, 0, 0, 47, 39, 0, G_TX_CLAMP, G_TX_CLAMP,
            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->panelBPalette);
        gSP1Quadrangle(gRegionAllocPtr++, 7, 6, 5, 4, 0);
    }
}
