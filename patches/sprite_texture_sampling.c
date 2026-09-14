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

extern s16 gMenuSpriteFlipScales[8];
extern u16 gMenuTransparentPalette[MENU_PALETTE_COLOR_COUNT];

RECOMP_PATCH void drawMenuSpriteClipped(s16 x, s16 y, AssetTable *table, u16 imageIndex, u16 scaleX, u16 scaleY,
                           u8 flipMode, u8 paletteIndex, s16 clipLeft, s16 clipTop, s16 clipRight,
                           s16 clipBottom) {
    AssetTableEntry *entry;
    s32 selectedPalette;
    u8 *palette;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 texS;
    s32 texT;
    u32 height;
    s32 pad;
    s16 flipS;
    s16 flipT;
    s16 pad2;
    s16 minX;
    s16 minY;
    s16 maxX;
    s16 maxY;

    entry = &table->entries[imageIndex];
    palette = table->entryCount * sizeof(AssetTableEntry) + (u8*)table->entries;
    if (scaleX > 0x200) {
        return;
    }
    if (scaleX <= 0) {
        return;
    }
    if (scaleY > 0x200) {
        return;
    }
    if (scaleY <= 0) {
        return;
    }

    flipS = gMenuSpriteFlipScales[(flipMode & 3) * 2 + 0];
    flipT = gMenuSpriteFlipScales[(flipMode & 3) * 2 + 1];

    texS = entry->width;
    texT = entry->height;

    left = (x + gMenuViewportCenterX) << 2;
    top = (y + gMenuViewportCenterY) << 2;
    right = left + (((scaleX * texS) << 2) >> 5);
    bottom = top + (((scaleY * texT) << 2) >> 5);

    height = entry->height;
    texS = 0 * height;
    texT = 0;
    if (flipS == -1) {
        texS = ((entry->width - 1) << 5);
    }
    if (flipT == -1) {
        texT = ((entry->height - 1) << 5) - texT;
    }


    clipTop = gMenuViewportCenterY - clipTop;
    clipBottom = gMenuViewportCenterY + clipBottom;
    clipLeft = gMenuViewportCenterX - clipLeft;
    clipRight = gMenuViewportCenterX + clipRight;
    if (clipLeft < gMenuViewportCenterX - (gMenuViewportWidth / 2)) {
        clipLeft = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    }
    if (clipRight > gMenuViewportCenterX + (gMenuViewportWidth / 2)) {
        clipRight = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    }
    if (clipTop < gMenuViewportCenterY - (gMenuViewportHeight / 2)) {
        clipTop = gMenuViewportCenterY - (gMenuViewportHeight / 2);
    }
    if (clipBottom > gMenuViewportCenterY + (gMenuViewportHeight / 2)) {
        clipBottom = gMenuViewportCenterY + (gMenuViewportHeight / 2);
    }

    minX = clipRight << 2;
    minY = clipBottom << 2;
    maxX = clipLeft << 2;
    maxY = clipTop << 2;
    if ((left >= minX) || (top >= minY) || (right < maxX) || (bottom < maxY)) {
        return;
    }

    if (left < maxX) {
        texS = (((maxX - left) << 3) << 5) / scaleX;
        if (flipS == -1) {
            texS = ((entry->width - 1) << 5) - texS;
        }
        left = maxX;
    }
    if (top < maxY) {
        texT = (((maxY - top) << 3) << 5) / scaleY;
        if (flipT == -1) {
            texT = ((entry->height - 1) << 5) - texT;
        }
        top = maxY;
    }
    if (right >= minX) {
        right = minX - 4;
    }
    if (bottom >= minY) {
        bottom = minY - 4;
    }

    if (paletteIndex == 0) {
        selectedPalette = entry->paletteIndex;
    } else {
        selectedPalette = (u16)(paletteIndex - 1);
    }

    gDPLoadTextureTile_4b(gRegionAllocPtr++, entry->imageOffset + (u8 *)table,
                          G_IM_FMT_CI, entry->width, entry->height, 0, 0,
                          // @recomp Clamp to the last texel so scaled confetti cannot sample an extra row or column.
                          entry->width - 1, entry->height - 1, 0, G_TX_CLAMP, G_TX_CLAMP,
                          G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    if (selectedPalette != 0xFE) {
        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, (selectedPalette << 5) + (u8*)palette);
    } else {
        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, gMenuTransparentPalette);
    }
    gSPTextureRectangle(gRegionAllocPtr++, left, top, right, bottom, G_TX_RENDERTILE,
                        texS, texT, (u16)((u16)(0x8000 / scaleX) * flipS),
                        (u16)((u16)(0x8000 / scaleY) * flipT));
}
