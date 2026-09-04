#include "patches.h"

#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/renderer/menu_renderer.h"

#define MENU_PALETTE_SIZE_BYTES 0x20
#define MENU_RGBA5551_ALPHA_BIT 1
#define MENU_RGBA5551_CHANNEL_MASK 0x1F
#define MENU_RGBA5551_SCALE_BASE 0x100
#define MENU_HALF_SCALE_STEP 0x800

extern Gfx* gRegionAllocPtr;
extern s16 gMenuViewportWidth;
extern s16 gMenuViewportHeight;
extern s16 gMenuViewportCenterX;
extern s16 gMenuViewportCenterY;

RECOMP_PATCH void drawMenuSpriteWithPaletteScale(s16 x, s16 y, AssetTable* asset, u16 index, u16 intensity) {
    AssetTableEntry* texture;
    u8* textureBase;
    u8* paletteBase;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 srcX;
    s32 srcY;
    u16 headerSize;
    MenuPalette* scratch;
    MenuPalette* source;
    u16 red;
    u16 green;
    u16 blue;
    u16 scaledRed;
    s32 i;

    headerSize = sizeof(AssetTableEntry);
    textureBase = (u8*) asset + (index * sizeof(AssetTableEntry));
    texture = (AssetTableEntry*) (textureBase + headerSize);
    paletteBase = (asset->entryCount * sizeof(AssetTableEntry)) + (u8*) asset + headerSize;
    left = x + gMenuViewportCenterX;
    top = y + gMenuViewportCenterY;
    right = left + (texture->width >> 1);
    bottom = top + (texture->height >> 1);
    srcX = 0;
    srcY = 0;

    if (left >= gMenuViewportCenterX + (gMenuViewportWidth / 2))
        return;
    if (top >= gMenuViewportCenterY + (gMenuViewportHeight / 2))
        return;
    if (right < gMenuViewportCenterX - (gMenuViewportWidth / 2))
        return;
    if (bottom < gMenuViewportCenterY - (gMenuViewportHeight / 2))
        return;
    if (left < gMenuViewportCenterX - (gMenuViewportWidth / 2)) {
        srcX = (gMenuViewportCenterX - (gMenuViewportWidth / 2)) - left;
        left = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    }
    if (top < gMenuViewportCenterY - (gMenuViewportHeight / 2)) {
        srcY = (gMenuViewportCenterY - (gMenuViewportHeight / 2)) - top;
        top = gMenuViewportCenterY - (gMenuViewportHeight / 2);
    }
    if (right >= gMenuViewportCenterX + (gMenuViewportWidth / 2))
        right = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    if (bottom >= gMenuViewportCenterY + (gMenuViewportHeight / 2))
        bottom = gMenuViewportCenterY + (gMenuViewportHeight / 2);

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureFilter(gRegionAllocPtr++, G_TF_AVERAGE);

    source = (MenuPalette*) (paletteBase + (texture->paletteIndex * MENU_PALETTE_SIZE_BYTES));
    scratch = allocMenuRenderScratch(sizeof(MenuPalette));
    for (i = 0; i != MENU_PALETTE_COLOR_COUNT; i++) {
        scratch->colors[i] = source->colors[i];
        if (scratch->colors[i] & MENU_RGBA5551_ALPHA_BIT) {
            red = (scratch->colors[i] >> 11) & MENU_RGBA5551_CHANNEL_MASK;
            green = (scratch->colors[i] >> 6) & MENU_RGBA5551_CHANNEL_MASK;
            blue = (scratch->colors[i] >> 1) & MENU_RGBA5551_CHANNEL_MASK;
            red = (red * intensity) / MENU_RGBA5551_SCALE_BASE;
            green = (green * intensity) / MENU_RGBA5551_SCALE_BASE;
            blue = (blue * intensity) / MENU_RGBA5551_SCALE_BASE;

            scratch->colors[i] = (red << 11) | (green << 6) | (blue << 1) | MENU_RGBA5551_ALPHA_BIT;
        }
    }

    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, scratch);
    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        texture->imageOffset + (u8*) asset,
        G_IM_FMT_CI,
        texture->width,
        texture->height,
        0,
        0,
        // @recomp Tile bounds are inclusive, so stop at the final texel instead of loading a wrapped column.
        texture->width - 1,
        // @recomp Tile bounds are inclusive, so stop at the final texel instead of loading a wrapped row.
        texture->height - 1,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gSPTextureRectangle(
        gRegionAllocPtr++,
        left << 2,
        top << 2,
        right << 2,
        bottom << 2,
        G_TX_RENDERTILE,
        // @recomp Start half a texel before the edge so half scaling averages the complete first texel pair.
        (srcX << 6) - 0x10,
        // @recomp Apply the same corrected half-scale origin and clipping offset vertically.
        (srcY << 6) - 0x10,
        MENU_HALF_SCALE_STEP,
        MENU_HALF_SCALE_STEP
    );
    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureFilter(gRegionAllocPtr++, G_TF_POINT);
    gDPPipeSync(gRegionAllocPtr++);
}

RECOMP_PATCH void drawScaledAssetTableSprite(s16 x, s16 y, AssetTable* asset, u16 entryIndex, u16 scale) {
    s32 x0;
    s32 y0;
    u8* paletteBase;
    s16 spriteWidth;
    s16 spriteHeight;
    s32 pad;
    s32 x1;
    s32 y1;
    s32 clippedS;
    s32 clippedT;
    AssetTableEntry* sprite;

    clippedS = scale;
    clippedT = scale;
    sprite = &asset->entries[entryIndex];
    x1 = spriteWidth >> clippedS; // fakematch
    if (scale < 0) {
        return;
    }
    paletteBase = asset->entryCount * sizeof(AssetTableEntry) + (u8*) asset->entries;

    x0 = x + gMenuViewportCenterX;
    y0 = y + gMenuViewportCenterY;
    spriteWidth = sprite->width;
    spriteHeight = sprite->height;
    x1 = spriteWidth >> clippedS;
    y1 = spriteHeight >> clippedT;
    x0 = x0 + ((spriteWidth - x1) / 2);
    y0 = y0 + ((spriteHeight - y1) / 2);
    x1 = x1 + x0;
    y1 = y1 + y0;

    clippedS = 0;
    clippedT = 0;

    if (x0 >= gMenuViewportCenterX + (gMenuViewportWidth / 2)) {
        return;
    }
    if (y0 >= gMenuViewportCenterY + (gMenuViewportHeight / 2)) {
        return;
    }
    if (x1 < gMenuViewportCenterX - (gMenuViewportWidth / 2)) {
        return;
    }
    if (y1 < gMenuViewportCenterY - (gMenuViewportHeight / 2)) {
        return;
    }
    if (x0 < gMenuViewportCenterX - (gMenuViewportWidth / 2)) {
        clippedS = (gMenuViewportCenterX - (gMenuViewportWidth / 2)) - x0;
        x0 = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    }
    if (y0 < gMenuViewportCenterY - (gMenuViewportHeight / 2)) {
        clippedT = (gMenuViewportCenterY - (gMenuViewportHeight / 2)) - y0;
        y0 = gMenuViewportCenterY - (gMenuViewportHeight / 2);
    }
    if (x1 >= gMenuViewportCenterX + (gMenuViewportWidth / 2)) {
        x1 = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    }
    if (y1 >= gMenuViewportCenterY + (gMenuViewportHeight / 2)) {
        y1 = gMenuViewportCenterY + (gMenuViewportHeight / 2);
    }

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureFilter(gRegionAllocPtr++, G_TF_AVERAGE);
    gDPLoadTextureTile_4b(gRegionAllocPtr++, sprite->imageOffset + (u8*) asset,
                          G_IM_FMT_CI, sprite->width, sprite->height,
                          0, 0,
                          // @recomp Tile bounds are inclusive, so exclude the wrapped column and row.
                          sprite->width - 1, sprite->height - 1, 0,
                          G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                          G_TX_NOLOD, G_TX_NOLOD);
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0,
                      (sprite->paletteIndex << 5) + paletteBase);
    gSPTextureRectangle(gRegionAllocPtr++, x0 << 2, y0 << 2,
                        x1 << 2, y1 << 2, G_TX_RENDERTILE,
                        // @recomp Scale clipping offsets and start half a texel before the horizontal source edge.
                        (clippedS << (scale + 5)) - 0x10,
                        // @recomp Scale clipping offsets and start half a texel before the vertical source edge.
                        (clippedT << (scale + 5)) - 0x10,
                        1 << (scale + 10),
                        1 << (scale + 10));
    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureFilter(gRegionAllocPtr++, G_TF_POINT);
    gDPPipeSync(gRegionAllocPtr++);
}

RECOMP_PATCH void drawScaledAssetTableSpriteWithExplicitPalette(s16 x, s16 y, AssetTable* asset, u16 entryIndex,
                                                                u16 paletteIndex, u16 scale) {
    s32 x0;
    s32 y0;
    u8* paletteBase;
    AssetTableEntry* sprite;
    s32 spriteWidth;
    s32 x1;
    s32 y1;
    s32 clippedS;
    s32 clippedT;
    s32 spriteHeight;

    clippedS = scale;
    clippedT = scale;
    sprite = &asset->entries[entryIndex];
    x1 = spriteWidth >> clippedS; // fakematch
    if (scale < 0) {
        return;
    }
    paletteBase = asset->entryCount * sizeof(AssetTableEntry) + (u8*) asset->entries;

    x0 = x + gMenuViewportCenterX;
    y0 = y + gMenuViewportCenterY;
    spriteWidth = sprite->width;
    spriteHeight = sprite->height;
    x1 = spriteWidth >> clippedS;
    y1 = spriteHeight >> clippedT;
    x0 = x0 + ((spriteWidth - x1) / 2);
    y0 = y0 + ((spriteHeight - y1) / 2);
    x1 = x1 + x0;
    y1 = y1 + y0;

    clippedS = 0;
    clippedT = 0;

    if (x0 >= gMenuViewportCenterX + (gMenuViewportWidth / 2)) {
        return;
    }

    if (y0 >= gMenuViewportCenterY + (gMenuViewportHeight / 2)) {
        return;
    }
    if (x1 < gMenuViewportCenterX - (gMenuViewportWidth / 2)) {
        return;
    }
    if (y1 < gMenuViewportCenterY - (gMenuViewportHeight / 2)) {
        return;
    }

    if (x0 < gMenuViewportCenterX - (gMenuViewportWidth / 2)) {
        clippedS = (gMenuViewportCenterX - (gMenuViewportWidth / 2)) - x0;
        x0 = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    }
    if (y0 < gMenuViewportCenterY - (gMenuViewportHeight / 2)) {
        clippedT = (gMenuViewportCenterY - (gMenuViewportHeight / 2)) - y0;
        y0 = gMenuViewportCenterY - (gMenuViewportHeight / 2);
    }
    if (x1 >= gMenuViewportCenterX + (gMenuViewportWidth / 2)) {
        x1 = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    }
    if (y1 >= gMenuViewportCenterY + (gMenuViewportHeight / 2)) {
        y1 = gMenuViewportCenterY + (gMenuViewportHeight / 2);
    }

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureFilter(gRegionAllocPtr++, G_TF_AVERAGE);
    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        sprite->imageOffset + (u8*) asset,
        G_IM_FMT_CI,
        sprite->width,
        sprite->height,
        0,
        0,
        // @recomp Tile bounds are inclusive, so stop at the final texel instead of loading a wrapped column.
        sprite->width - 1,
        // @recomp Tile bounds are inclusive, so stop at the final texel instead of loading a wrapped row.
        sprite->height - 1,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, (paletteIndex << 5) + paletteBase);
    gSPTextureRectangle(
        gRegionAllocPtr++,
        x0 << 2,
        y0 << 2,
        x1 << 2,
        y1 << 2,
        G_TX_RENDERTILE,
        // @recomp Scale clipping offsets and start half a texel before the horizontal source edge.
        (clippedS << (scale + 5)) - 0x10,
        // @recomp Scale clipping offsets and start half a texel before the vertical source edge.
        (clippedT << (scale + 5)) - 0x10,
        1 << (scale + 10),
        1 << (scale + 10)
    );
    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureFilter(gRegionAllocPtr++, G_TF_POINT);
    gDPPipeSync(gRegionAllocPtr++);
}
