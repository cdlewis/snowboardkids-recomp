#include "patches.h"

#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/ui/race_ui_effects.h"

// This actor is private to race_ui_effects.c in the decompilation. Keep its layout
// and field names identical; the image/palette field names are reversed there.
typedef struct RaceUiOverlayActor {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ Vec3i pos;
    /* 0x24 */ u8 pad24[4];
    /* 0x28 */ s32 velocity;
    /* 0x2C */ u8 pad2C[4];
    /* 0x30 */ s16 timer;
    /* 0x32 */ s16 assetTimer;
    /* 0x34 */ Mtx *matrix;
    /* 0x38 */ void *image3A;
    /* 0x3C */ void *palette3A;
    /* 0x40 */ void *image3B;
    /* 0x44 */ void *palette3B;
    /* 0x48 */ u8 matrixDirty;
} RaceUiOverlayActor;

extern Gfx *gRegionAllocPtr;
extern Gfx gAlphaSpriteRenderModeDl[];
extern Mtx *gViewportMatrix;
extern u8 gRenderMatricesDirty;
extern Vtx D_800D69A8[];

static void loadRaceStartSpritePanel(void *image, void *palette) {
    // The two quads span 48x40 texels. LoadTile and SetTileSize endpoints are
    // inclusive: the original (48, 40) reads a spare column/row and lets filtering
    // sample them along the balloon edge. Retain the original half-texel UVs.
    gDPLoadTextureTile_4b(
        gRegionAllocPtr++, image, G_IM_FMT_CI, 48, 40, 0, 0, 47, 39, 0,
        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD
    );
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, palette);
}

RECOMP_PATCH void func_80065808(RaceUiOverlayActor *actor) {
    Transform3D transform;

    if (gRenderMatricesDirty != 0) {
        actor->matrixDirty = 1;
    }
    if (actor->matrixDirty != 0) {
        actor->matrixDirty = 0;
        transform = gIdentityFixedTransform;
        transform.translation.x = actor->pos.x;
        transform.translation.y = actor->pos.y;
        transform.translation.z = actor->pos.z;
        actor->matrix = allocFixedTransformMatrix(&transform);
    }
    if (actor->matrix == NULL) {
        return;
    }

    // Preserve the original load, matrix, vertex and panel draw order. Animation
    // continues to select the lower panel's frames in func_80065D24.
    gSPDisplayList(gRegionAllocPtr++, gAlphaSpriteRenderModeDl);
    loadRaceStartSpritePanel(actor->palette3A, actor->image3A);
    gSPMatrix(gRegionAllocPtr++, actor->matrix, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gRegionAllocPtr++, gViewportMatrix, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPVertex(gRegionAllocPtr++, D_800D69A8, 8, 0);
    gSP1Quadrangle(gRegionAllocPtr++, 3, 2, 1, 0, 0);
    loadRaceStartSpritePanel(actor->palette3B, actor->image3B);
    gSP1Quadrangle(gRegionAllocPtr++, 7, 6, 5, 4, 0);
}
