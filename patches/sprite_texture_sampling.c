#include "patches.h"

#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"

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

RECOMP_PATCH void drawAssetTableSprite(s16 x, s16 y, AssetTable *table, u16 entryIndex) {
    AssetTableEntry *entry;
    s32 maxX;
    u8 *paletteBase;
    s32 maxY;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    s32 minY;
    s32 x0;
    s32 minX;
    s32 halfHeight;

    paletteBase = (table->entryCount * sizeof(AssetTableEntry)) + (u8 *)table + sizeof(AssetTableEntry);
    entry = &table->entries[entryIndex];
    x0 = x + gMenuViewportCenterX;
    entry += 0;
    y0 = y + gMenuViewportCenterY;
    x1 = x0 + entry->width;
    y1 = y0 + entry->height;
    clipS = 0;
    clipT = 0;

    maxX = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    if (x0 >= maxX) {
        return;
    }

    halfHeight = gMenuViewportHeight / 2;
    maxY = gMenuViewportCenterY + halfHeight;
    minX = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    if (y0 >= maxY) {
        return;
    }
    if (x1 < minX) {
        return;
    }

    minY = gMenuViewportCenterY - halfHeight;
    if (y1 < minY) {
        return;
    }

    if (x0 < minX) {
        clipS = minX - x0;
        x0 = minX;
    }
    if (y0 < minY) {
        clipT = minY - y0;
        y0 = minY;
    }
    if (x1 >= maxX) {
        x1 = maxX;
    }
    if (y1 >= maxY) {
        y1 = maxY;
    }

    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        entry->imageOffset + (u8 *)table,
        G_IM_FMT_CI,
        entry->width,
        entry->height,
        0,
        0,
        // @recomp Exclude the extra row and column from the inclusive load bounds.
        entry->width - 1,
        entry->height - 1,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, paletteBase + (entry->paletteIndex << 5));
    gSPTextureRectangle(
        gRegionAllocPtr++,
        x0 << 2,
        y0 << 2,
        x1 << 2,
        y1 << 2,
        G_TX_RENDERTILE,
        clipS << 5,
        clipT << 5,
        0x400,
        0x400
    );
}

RECOMP_PATCH void drawPulsingAssetTableSprite(s16 x, s16 y, AssetTable *table, u16 entryIndex) {
    AssetTableEntry *entry;
    s32 maxX;
    u8 *paletteBase;
    s32 maxY;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    s32 minY;
    s32 x0;
    s32 minX;
    s32 halfHeight;
    s32 pulse;

    paletteBase = (table->entryCount * sizeof(AssetTableEntry)) + (u8 *)table + sizeof(AssetTableEntry);
    entry = &table->entries[entryIndex];
    x0 = x + gMenuViewportCenterX;
    entry += 0;
    y0 = y + gMenuViewportCenterY;
    x1 = x0 + entry->width;
    y1 = y0 + entry->height;
    clipS = 0;
    clipT = 0;

    maxX = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    if (x0 >= maxX) {
        return;
    }

    halfHeight = gMenuViewportHeight / 2;
    maxY = gMenuViewportCenterY + halfHeight;

    if (1) {
        minX = gMenuViewportCenterX - (gMenuViewportWidth / 2);
        if (y0 >= maxY) {
            return;
        }
        if (x1 < minX) {
            return;
        }
    }

    minY = gMenuViewportCenterY - halfHeight;
    if (y1 < minY) {
        return;
    }

    if (x0 < minX) {
        clipS = minX - x0;
        x0 = minX;
    }
    if (y0 < minY) {
        clipT = minY - y0;
        y0 = minY;
    }
    if (x1 >= maxX) {
        x1 = maxX;
    }
    if (y1 >= maxY) {
        y1 = maxY;
    }

    pulse = gFrameCounter & 0x1F;
    if (pulse >= 0x11) {
        pulse = 0x20 - pulse;
    }
    pulse *= 0x10;
    if (pulse >= 0x100) {
        pulse = 0xFF;
    }

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetCombineMode(gRegionAllocPtr++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0xFF, 0xFF, pulse, 0xFF);
    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        entry->imageOffset + (u8 *)table,
        G_IM_FMT_CI,
        entry->width,
        entry->height,
        0,
        0,
        // @recomp Exclude the extra row and column from the inclusive load bounds.
        entry->width - 1,
        entry->height - 1,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, paletteBase + (entry->paletteIndex << 5));
    gSPTextureRectangle(
        gRegionAllocPtr++,
        x0 << 2,
        y0 << 2,
        x1 << 2,
        y1 << 2,
        G_TX_RENDERTILE,
        clipS << 5,
        clipT << 5,
        0x400,
        0x400
    );
    gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
}

RECOMP_PATCH void drawAssetTableSpriteWithDefaultPalette(s16 x, s16 y, AssetTable *table, u16 entryIndex) {
    AssetTableEntry *entry;
    s32 maxX;
    s32 minY;
    s32 maxY;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    s32 x0;
    s32 minX;
    s32 halfHeight;

    entry = &table->entries[entryIndex];
    x0 = x + gMenuViewportCenterX;
    entry += 0;
    y0 = y + gMenuViewportCenterY;
    x1 = x0 + entry->width;
    y1 = y0 + entry->height;
    clipS = 0;
    clipT = 0;

    maxX = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    if (x0 >= maxX) {
        return;
    }

    halfHeight = gMenuViewportHeight / 2;
    maxY = gMenuViewportCenterY + halfHeight;
    minX = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    if (y0 >= maxY) {
        return;
    }
    if (x1 < minX) {
        return;
    }

    minY = gMenuViewportCenterY - halfHeight;
    if (y1 < minY) {
        return;
    }

    if (x0 < minX) {
        clipS = minX - x0;
        x0 = minX;
    }
    if (y0 < minY) {
        clipT = minY - y0;
        y0 = minY;
    }
    if (x1 >= maxX) {
        x1 = maxX;
    }
    if (y1 >= maxY) {
        y1 = maxY;
    }

    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        entry->imageOffset + (u8 *)table,
        G_IM_FMT_CI,
        entry->width,
        entry->height,
        0,
        0,
        // @recomp Exclude the extra row and column from the inclusive load bounds.
        entry->width - 1,
        entry->height - 1,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, D_800D40B0);
    gSPTextureRectangle(
        gRegionAllocPtr++,
        x0 << 2,
        y0 << 2,
        x1 << 2,
        y1 << 2,
        G_TX_RENDERTILE,
        clipS << 5,
        clipT << 5,
        0x400,
        0x400
    );
}

RECOMP_PATCH void drawAssetTableSprite8bpp(s16 x, s16 y, AssetTable *table, u16 entryIndex) {
    AssetTableEntry *entry;
    s32 maxX;
    u8 *paletteBase;
    s32 maxY;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    s32 minY;
    s32 x0;
    s32 minX;
    s32 halfHeight;

    paletteBase = (table->entryCount * sizeof(AssetTableEntry)) + (u8 *)table + sizeof(AssetTableEntry);
    entry = &table->entries[entryIndex];
    x0 = x + gMenuViewportCenterX;
    entry += 0;
    y0 = y + gMenuViewportCenterY;
    x1 = x0 + entry->width;
    y1 = y0 + entry->height;
    clipS = 0;
    clipT = 0;

    maxX = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    if (x0 >= maxX) {
        return;
    }

    halfHeight = gMenuViewportHeight / 2;
    maxY = gMenuViewportCenterY + halfHeight;
    minX = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    if (y0 >= maxY) {
        return;
    }
    if (x1 < minX) {
        return;
    }

    minY = gMenuViewportCenterY - halfHeight;
    if (y1 < minY) {
        return;
    }

    if (x0 < minX) {
        clipS = minX - x0;
        x0 = minX;
    }
    if (y0 < minY) {
        clipT = minY - y0;
        y0 = minY;
    }
    if (x1 >= maxX) {
        x1 = maxX;
    }
    if (y1 >= maxY) {
        y1 = maxY;
    }

    gDPLoadTextureTile(
        gRegionAllocPtr++,
        entry->imageOffset + (u8 *)table,
        G_IM_FMT_CI,
        G_IM_SIZ_8b,
        entry->width,
        entry->height,
        0,
        0,
        // @recomp Exclude the extra row and column from the inclusive load bounds.
        entry->width - 1,
        entry->height - 1,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPLoadTLUT_pal256(gRegionAllocPtr++, paletteBase + (entry->paletteIndex << 5));
    gSPTextureRectangle(
        gRegionAllocPtr++,
        x0 << 2,
        y0 << 2,
        x1 << 2,
        y1 << 2,
        G_TX_RENDERTILE,
        clipS << 5,
        clipT << 5,
        0x400,
        0x400
    );
}

RECOMP_PATCH void drawAssetTableSpriteWithExplicitPalette(s16 x, s16 y, AssetTable *table, u16 entryIndex, u16 paletteIndex) {
    AssetTableEntry *entry;
    s32 maxX;
    u8 *paletteBase;
    s32 maxY;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    s32 minY;
    s32 x0;
    s32 minX;
    s32 halfHeight;

    paletteBase = (table->entryCount * sizeof(AssetTableEntry)) + (u8 *)table + sizeof(AssetTableEntry);
    entry = &table->entries[entryIndex];
    x0 = x + gMenuViewportCenterX;
    entry += 0;
    y0 = y + gMenuViewportCenterY;
    x1 = x0 + entry->width;
    y1 = y0 + entry->height;
    clipS = 0;
    clipT = 0;

    maxX = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    if (x0 >= maxX) {
        return;
    }

    halfHeight = gMenuViewportHeight / 2;
    maxY = gMenuViewportCenterY + halfHeight;
    minX = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    if (y0 >= maxY) {
        return;
    }
    if (x1 < minX) {
        return;
    }

    minY = gMenuViewportCenterY - halfHeight;
    if (y1 < minY) {
        return;
    }

    if (x0 < minX) {
        clipS = minX - x0;
        x0 = minX;
    }
    if (y0 < minY) {
        clipT = minY - y0;
        y0 = minY;
    }
    if (x1 >= maxX) {
        x1 = maxX;
    }
    if (y1 >= maxY) {
        y1 = maxY;
    }

    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        entry->imageOffset + (u8 *)table,
        G_IM_FMT_CI,
        entry->width,
        entry->height,
        0,
        0,
        // @recomp Exclude the extra row and column from the inclusive load bounds.
        entry->width - 1,
        entry->height - 1,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, paletteBase + (paletteIndex << 5));
    gSPTextureRectangle(
        gRegionAllocPtr++,
        x0 << 2,
        y0 << 2,
        x1 << 2,
        y1 << 2,
        G_TX_RENDERTILE,
        clipS << 5,
        clipT << 5,
        0x400,
        0x400
    );
}

RECOMP_PATCH void drawMenuAsciiCharImpl(s16 x, s16 y, u8 ch, u16 arg3) {
    char pad[8];
    u32 tile;
    s16 s;
    MenuAsciiFontAsset *font;

    if ((ch >= 'a') && (ch <= 'z')) {
        if (gMenuAsciiFontTextureNeedsLoad) {
            font = getRelocatableHeapBlockBase(gAssetHandles[6]);

            gDPLoadTextureTile_4b(
                gRegionAllocPtr++,
                font->imageOffset + (u8 *)font,
                G_IM_FMT_CI,
                font->width,
                font->height,
                0,
                0,
                // @recomp Exclude the extra row and column from the inclusive load bounds.
                font->width - 1,
                font->height - 1,
                0,
                G_TX_CLAMP,
                G_TX_CLAMP,
                G_TX_NOMASK,
                G_TX_NOMASK,
                G_TX_NOLOD,
                G_TX_NOLOD
            );

            gMenuAsciiFontTextureNeedsLoad = 0;
            gMenuAsciiFontPaletteIndex = -1;
        }
        tile = ch - 0x40;
        s = ((tile & 7) << 3);
        drawMenuAsciiFontTile(x, y, s, (s16)(tile & 0x38), arg3);
    } else {
        if (gMenuAsciiFontTextureNeedsLoad != 0) {
            font = getRelocatableHeapBlockBase(gAssetHandles[6]);

            gDPLoadTextureTile_4b(
                gRegionAllocPtr++,
                font->imageOffset + (u8 *)font,
                G_IM_FMT_CI,
                font->width,
                font->height,
                0,
                0,
                // @recomp Exclude the extra row and column from the inclusive load bounds.
                font->width - 1,
                font->height - 1,
                0,
                G_TX_CLAMP,
                G_TX_CLAMP,
                G_TX_NOMASK,
                G_TX_NOMASK,
                G_TX_NOLOD,
                G_TX_NOLOD
            );

            gMenuAsciiFontTextureNeedsLoad = 0;
            gMenuAsciiFontPaletteIndex = -1;
        }
        tile = ch - 0x20;
        if (tile < 0x40) {
            s = ((tile & 7) << 3);
            drawMenuAsciiFontTile(x, y, s, (s16)(tile & 0x38), arg3);
        }
    }
}

RECOMP_PATCH void drawMenuSpriteWithAlphaClipped(
    s16 x,
    s16 y,
    AssetTable *asset,
    u16 tileIndex,
    u16 scaleX,
    u16 scaleY,
    u8 flipMode,
    u16 alpha,
    u8 paletteIndex,
    s32 clipLeft,
    s32 clipTop,
    s32 clipRight,
    s32 clipBottom
) {
    AssetTableEntry *texture;
    volatile u8 paddingA[4];
    u8 *paletteBase;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 texS;
    s32 texT;
    u32 height;
    volatile u8 paddingB[4];
    s32 paddingWord;
    s16 flipS;
    s16 flipT;
    s16 paddingHalfword;
    s32 minX;
    s32 minY;
    s32 maxX;
    s32 maxY;
    volatile u16 paddingC;
    u16 selectedPalette;

    texture = &asset->entries[tileIndex];
    paletteBase = (asset->entryCount * sizeof(AssetTableEntry)) + (u8 *)asset + sizeof(AssetTableEntry);
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
    flipS = ((s16 *)gMenuSpriteFlipScales)[(flipMode & 3) * 2 + 0];
    flipT = ((s16 *)gMenuSpriteFlipScales)[(flipMode & 3) * 2 + 1];

    texS = texture->width;
    texT = texture->height;
    left = (x + gMenuViewportCenterX) << 2;
    top = (y + gMenuViewportCenterY) << 2;
    right = left + (((scaleX * texS) << 2) >> 5);
    bottom = top + (((scaleY * texT) << 2) >> 5);

    height = texture->height;

    texS = height * 0;
    texT = 0;
    if (flipS == -1) {
        texS = (texture->width - 1) << 5;
    }
    if (flipT == -1) {
        texT = ((texture->height - 1) << 5) - texT;
    }

    minY = (s16)((gMenuViewportCenterY - (s16)clipTop) << 2);
    maxY = (s16)((gMenuViewportCenterY + (s16)clipBottom) << 2);
    minX = (s16)((gMenuViewportCenterX - (s16)clipLeft) << 2);
    maxX = (s16)((gMenuViewportCenterX + (s16)clipRight) << 2);

    if ((left >= maxX) || (top >= maxY) || (right < minX) || (bottom < minY)) {
        return;
    }
    if (left < minX) {
        texS = (((minX - left) << 3) << 5) / scaleX;
        if (flipS == -1) {
            texS = ((texture->width - 1) << 5) - texS;
        }
        left = minX;
    }
    if (top < minY) {
        texT = (((minY - top) << 3) << 5) / scaleY;
        if (flipT == -1) {
            texT = ((texture->height - 1) << 5) - texT;
        }
        top = minY;
    }
    if (right >= maxX) {
        right = maxX - 4;
    }
    if (bottom >= maxY) {
        bottom = maxY - 4;
    }

    if (paletteIndex == 0) {
        selectedPalette = texture->paletteIndex;
    } else {
        selectedPalette = paletteIndex - 1;
    }

    if (alpha != 0x100) {
        gDPPipeSync(gRegionAllocPtr++);
        gDPSetCombineMode(gRegionAllocPtr++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(gRegionAllocPtr++, 0, 0, alpha, alpha, alpha, 0xFF);
    }

    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        texture->imageOffset + (u8 *)asset,
        G_IM_FMT_CI,
        texture->width,
        texture->height,
        0,
        0,
        // @recomp Exclude the extra row and column from the inclusive load bounds.
        texture->width - 1,
        texture->height - 1,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, paletteBase + (selectedPalette << 5));
    gSPTextureRectangle(
        gRegionAllocPtr++,
        left,
        top,
        right,
        bottom,
        G_TX_RENDERTILE,
        texS,
        texT,
        (u16)((u16)(0x8000 / scaleX) * flipS),
        (u16)((u16)(0x8000 / scaleY) * flipT)
    );
    if (alpha != 0x100) {
        gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
    }
}

RECOMP_PATCH void drawMenuSpriteSubrect(
    s16 x,
    s16 y,
    AssetTable *asset,
    u16 index,
    u8 srcX,
    u8 srcY,
    u8 width,
    u8 height,
    s32 scaleX,
    s32 scaleY
) {
    AssetTableEntry *texture;
    s32 minX;
    u8 *paletteBase;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 texS;
    s32 texT;
    s32 minY;
    u16 scaleXValue;
    u16 scaleYValue;
    s16 maxX;
    s16 maxY;

    texture = &asset->entries[index];
    paletteBase = (asset->entryCount * sizeof(AssetTableEntry)) + (u8 *)asset + sizeof(AssetTableEntry);
    scaleXValue = scaleX;
    scaleYValue = scaleY;
    left = (x + gMenuViewportCenterX) << 2;
    top = (y + gMenuViewportCenterY) << 2;
    right = (((width * scaleXValue) << 2) >> 5) + left;
    bottom = height;
    bottom *= scaleYValue;
    bottom = ((bottom << 2) >> 5) + top;

    texS = srcX << 5;
    texT = srcY << 5;
    minY = (s16)((gMenuViewportCenterY - (gMenuViewportHeight / 2)) << 2);
    maxY = (gMenuViewportCenterY + (gMenuViewportHeight / 2)) << 2;
    minX = (s16)((gMenuViewportCenterX - (gMenuViewportWidth / 2)) << 2);
    maxX = (gMenuViewportCenterX + (gMenuViewportWidth / 2)) << 2;

    if ((left < maxX) && (top < maxY) && (right >= minX) && (bottom >= minY)) {
        if (left < minX) {
            do {
                texS = ((((minX - left) << 3) << 5) / scaleXValue) + texS;
                left = minX;
            } while (0);
        }
        if (top < minY) {
            texT = ((((minY - top) << 3) << 5) / scaleYValue) + texT;
            top = minY;
        }
        if (right >= maxX) {
            right = maxX - 4;
        }
        if (bottom >= maxY) {
            bottom = maxY - 4;
        }

        gDPLoadTextureTile_4b(
            gRegionAllocPtr++,
            texture->imageOffset + (u8 *)asset + 0x80000000,
            G_IM_FMT_CI,
            texture->width,
            texture->height,
            0,
            0,
            // @recomp Exclude the extra row and column from the inclusive load bounds.
            texture->width - 1,
            texture->height - 1,
            0,
            G_TX_CLAMP,
            G_TX_CLAMP,
            G_TX_NOMASK,
            G_TX_NOMASK,
            G_TX_NOLOD,
            G_TX_NOLOD
        );
        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, paletteBase + (texture->paletteIndex << 5) + 0x80000000);
        gSPTextureRectangle(
            gRegionAllocPtr++,
            left,
            top,
            right,
            bottom,
            G_TX_RENDERTILE,
            texS,
            texT,
            (u16)(0x8000 / scaleXValue),
            (u16)(0x8000 / scaleYValue)
        );
    }
}

RECOMP_PATCH void drawMenuSpriteTileClipped(
    s16 x,
    s16 y,
    AssetTable *table,
    u16 entryIndex,
    u16 unused,
    u16 intensity,
    s16 clipX,
    s16 clipY
) {
    AssetTableEntry *entry;
    volatile s32 padding2;
    u8 *paletteBase;
    volatile u8 padding0[0x18];
    s32 y0;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    volatile u8 padding1[8];
    s32 x0;
    s16 minX;
    s16 minY;
    s16 maxX;
    s16 maxY;

    paletteBase = (u8 *)(table->entryCount + table->entries);
    entry = &table->entries[entryIndex];
    entry += 0;
    clipS = entry->width;
    x1 = x0 = x + gMenuViewportCenterX;
    y0 = y + gMenuViewportCenterY;
    x1 += clipS;
    y1 = y0 + entry->height;
    clipS = 0;
    clipT = 0;
    minX = gMenuViewportCenterX - clipX;
    maxX = gMenuViewportCenterX + clipX;
    minY = gMenuViewportCenterY - clipY;
    maxY = gMenuViewportCenterY + clipY;

    if (minX < gMenuViewportCenterX - (gMenuViewportWidth / 2)) {
        minX = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    }
    if (gMenuViewportCenterX + (gMenuViewportWidth / 2) < maxX) {
        maxX = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    }
    if (minY < gMenuViewportCenterY - (gMenuViewportHeight / 2)) {
        minY = gMenuViewportCenterY - (gMenuViewportHeight / 2);
    }
    if (gMenuViewportCenterY + (gMenuViewportHeight / 2) < maxY) {
        maxY = gMenuViewportCenterY + (gMenuViewportHeight / 2);
    }

    if (x0 >= maxX) {
        return;
    }
    if (y0 >= maxY) {
        return;
    }
    if (x1 < minX) {
        return;
    }
    if (y1 < minY) {
        return;
    }

    if (x0 < minX) {
        clipS = minX - x0;
        x0 = minX;
    }
    if (y0 < minY) {
        clipT = minY - y0;
        y0 = minY;
    }
    if (x1 >= maxX) {
        x1 = maxX - 1;
    }
    if (y1 >= maxY) {
        y1 = maxY - 1;
    }

    gDPLoadTextureTile(
        gRegionAllocPtr++,
        entry->imageOffset + (u8 *)table,
        G_IM_FMT_CI,
        G_IM_SIZ_8b,
        entry->width,
        entry->height,
        0,
        0,
        // @recomp Exclude the extra row and column from the inclusive load bounds.
        entry->width - 1,
        entry->height - 1,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    if (intensity != 0x100) {
        gDPPipeSync(gRegionAllocPtr++);
        gDPSetCombineMode(gRegionAllocPtr++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(gRegionAllocPtr++, 0, 0, intensity, intensity, intensity, 0xFF);
    }
    gDPLoadTLUT_pal256(gRegionAllocPtr++, paletteBase + (entry->paletteIndex << 5));
    gSPTextureRectangle(
        gRegionAllocPtr++,
        x0 << 2,
        y0 << 2,
        x1 << 2,
        y1 << 2,
        G_TX_RENDERTILE,
        clipS << 5,
        clipT << 5,
        0x400,
        0x400
    );
    if (intensity != 0x100) {
        gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
    }
}

RECOMP_PATCH void drawMenuGlyph(s16 x, s16 y, u16 glyphIndex, u8 paletteIndex, u16 intensity, u16 fontBank) {
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    s32 color;
    s32 i;
    u16 paletteColor;
    u8 new_var;
    MenuPalette *paletteBase;
    AssetTable *font;
    MenuPalette *srcPalette;
    u16 *scaledPalette;
    AssetTableEntry *entry;
    u16 red;
    u16 green;
    u16 blue;

    if (paletteIndex == 0) {
        font = getRelocatableHeapBlockBase(gAssetHandles[fontBank]);
        color = 0x10;
    } else {
        font = getRelocatableHeapBlockBase(gAssetHandles[fontBank + 1]);
        color = 8;
    }

    paletteBase = (MenuPalette *)(font->entryCount + font->entries);
    x1 = x0 = x + gMenuViewportCenterX;
    y0 = y + gMenuViewportCenterY;
    x1 = color + x0;
    y1 = y0 + 0x10;
    clipS = 0;
    clipT = 0;

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
        clipS = (gMenuViewportCenterX - (gMenuViewportWidth / 2)) - x0;
        x0 = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    }
    if (y0 < gMenuViewportCenterY - (gMenuViewportHeight / 2)) {
        clipT = (gMenuViewportCenterY - (gMenuViewportHeight / 2)) - y0;
        y0 = gMenuViewportCenterY - (gMenuViewportHeight / 2);
    }
    if (x1 >= gMenuViewportCenterX + (gMenuViewportWidth / 2)) {
        x1 = (gMenuViewportCenterX + (gMenuViewportWidth / 2)) - 1;
    }
    if (y1 >= gMenuViewportCenterY + (gMenuViewportHeight / 2)) {
        y1 = (gMenuViewportCenterY + (gMenuViewportHeight / 2)) - 1;
    }

    entry = font->entries;
    entry += glyphIndex;
    srcPalette = &paletteBase[entry->paletteIndex];
    scaledPalette = allocMenuRenderScratch(0x20);
    for (i = 0; i < 0x10; i++) {
        scaledPalette[i] = srcPalette->colors[i];

        if (scaledPalette[i] & 1) {
            red = (scaledPalette[i] >> 11) & 0x1F;
            green = (scaledPalette[i] >> 6) & 0x1F;
            blue = (scaledPalette[i] >> 1) & 0x1F;
            red = (red * intensity) / 0x100;
            green = (green * intensity) / 0x100;
            blue = (blue * intensity) / 0x100;
            scaledPalette[i] = (((red << 11) | (green << 6)) | (blue << 1)) | 1;
        }
    }

    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, scaledPalette);
    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        entry->imageOffset + (u8 *)font,
        G_IM_FMT_CI,
        entry->width,
        entry->height,
        0,
        0,
        // @recomp Exclude the extra row and column from the inclusive load bounds.
        entry->width - 1,
        entry->height - 1,
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
        x0 << 2,
        y0 << 2,
        x1 << 2,
        y1 << 2,
        G_TX_RENDERTILE,
        clipS << 5,
        clipT << 5,
        0x400,
        0x400
    );
}

RECOMP_PATCH void drawMenuColoredGlyph(s16 x, s16 y, u16 glyph, u8 palette, u16 paletteScale, u16 paletteIndex, s32 fontBank) {
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    s32 glyphWidth;
    s32 i;
    u16 paletteColor;
    u8 new_var;
    MenuPalette *paletteBase;
    AssetTable *font;
    MenuPalette *srcPalette;
    u16 *scaledPalette;
    AssetTableEntry *entry;
    s32 color;
    u16 red;
    u16 green;
    u16 blue;
    u16 paletteScaleValue;
    s32 viewHalfWidth;
    s32 viewHalfHeight;
    s32 minX;
    s32 maxX;
    s32 minY;
    s32 viewHalfHeightValue;
    s32 maxY;
    u16 paletteIndexValue;

    if (palette == 0) {
        font = getRelocatableHeapBlockBase(gAssetHandles[(u16)fontBank]);
        glyphWidth = 0x10;
    } else {
        font = getRelocatableHeapBlockBase(gAssetHandles[((u16)fontBank) + 1]);
        glyphWidth = 8;
    }
    paletteBase = (MenuPalette *)(font->entryCount + font->entries);
    srcPalette = &paletteBase[paletteIndex];
    x0 = x + gMenuViewportCenterX;
    y0 = y + gMenuViewportCenterY;
    x1 = glyphWidth + x0;
    y1 = y0 + 0x10;
    clipS = 0;
    clipT = 0;
    viewHalfWidth = gMenuViewportWidth / 2;
    maxX = gMenuViewportCenterX + viewHalfWidth;
    if (x0 < maxX) {
        viewHalfHeightValue = gMenuViewportHeight / 2;
        minX = gMenuViewportCenterX - viewHalfWidth;
        maxY = gMenuViewportCenterY + viewHalfHeightValue;
        if ((y0 < maxY) && (x1 >= minX)) {
            minY = gMenuViewportCenterY - viewHalfHeightValue;
            if (y1 >= minY) {
                if (x0 < minX) {
                    clipS = minX - x0;
                    x0 = minX;
                }
                if (y0 < minY) {
                    clipT = minY - y0;
                    y0 = minY;
                }
                if (x1 >= maxX) {
                    x1 = maxX - 1;
                }
                if (y1 >= maxY) {
                    y1 = maxY - 1;
                }
                scaledPalette = allocMenuRenderScratch(0x20);
                for (i = 0; i < 0x10; i++) {
                    scaledPalette[i] = srcPalette->colors[i];

                    if (scaledPalette[i] & 1) {
                        red = (scaledPalette[i] >> 11) & 0x1F;
                        green = (scaledPalette[i] >> 6) & 0x1F;
                        blue = (scaledPalette[i] >> 1) & 0x1F;
                        red = (red * paletteScale) / 0x100;
                        green = (green * paletteScale) / 0x100;
                        blue = (blue * paletteScale) / 0x100;
                        scaledPalette[i] = (((red << 11) | (green << 6)) | (blue << 1)) | 1;
                    }
                }

                entry = &font->entries[glyph];
                if (1) {}

                gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, scaledPalette);
                gDPLoadTextureTile_4b(
                    gRegionAllocPtr++,
                    entry->imageOffset + (u8 *)font,
                    2,
                    entry->width,
                    entry->height,
                    0,
                    0,
                    // @recomp Exclude the extra row and column from the inclusive load bounds.
                    entry->width - 1,
                    entry->height - 1,
                    0,
                    0x2,
                    0x2,
                    0,
                    0,
                    0,
                    0
                );
                gSPTextureRectangle(
                    gRegionAllocPtr++,
                    x0 << 2,
                    y0 << 2,
                    x1 << 2,
                    y1 << 2,
                    0,
                    clipS << 5,
                    clipT << 5,
                    0x400,
                    0x400
                );
            }
        }
    }
}

RECOMP_PATCH void drawMenuAsciiGlyph(s16 x, s16 y, u16 tileS, s32 tileT, u16 palette, u16 paletteScale) {
    s32 x0;
    s32 storedY;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    s32 maxX;
    volatile u16 *dst;
    AssetTableEntry *texture;
    MenuPalette *palettes;
    AssetTable *font;
    s32 maxY;
    s32 i;
    s16 paletteIndex;

    font = getRelocatableHeapBlockBase(gAssetHandles[6]);
    palettes = (MenuPalette *)(font->entryCount + font->entries);
    paletteIndex = *(s16 *)&font->entries[0].paletteIndex;
    x0 = x + gMenuViewportCenterX;
    i = y + gMenuViewportCenterY;
    texture = &font->entries[0];
    x1 = x0 + 8;
    y1 = i + 8;
    clipS = 0;
    clipT = 0;

    {
        s32 minX;
        s32 minY;
        s32 halfHeight;

        maxX = gMenuViewportCenterX + (gMenuViewportWidth / 2);
        if (x0 < maxX) {
            halfHeight = gMenuViewportHeight / 2;
            maxY = gMenuViewportCenterY + halfHeight;
            minX = gMenuViewportCenterX - (gMenuViewportWidth / 2);
            if (i < maxY) {
                minY = gMenuViewportCenterY - halfHeight;
                if ((x1 >= minX) && (y1 >= minY)) {
                    s32 colorValue;
                    MenuPalette *scratch;
                    MenuPalette *source;
                    s32 color;
                    s32 red;
                    u16 green;
                    u16 blue;
                    s32 scaleValue;
                    u16 scaledRed;

                    if (x0 < minX) {
                        clipS = minX - x0;
                        x0 = minX;
                    }
                    if (i < minY) {
                        clipT = minY - i;
                        i = minY;
                    }
                    if (x1 >= maxX) {
                        x1 = maxX - 1;
                    }
                    if (y1 >= maxY) {
                        y1 = maxY - 1;
                    }
                    clipS += tileS;
                    clipT += (u16)tileT;

                    if (paletteIndex != palette) {
                        paletteIndex = palette;
                    }
                    storedY = i;
                    if (1) {
                        scratch = allocMenuRenderScratch(sizeof(MenuPalette)); \
                        i = 0; \
                        source = &palettes[paletteIndex]; \
                        dst = scratch->colors; \
                    palette_loop: \
                        *dst = (color = (*(u16 *)&source->bytes[i]) & 0xFFFFu); \
                        i += sizeof(u16); \
                        if ((colorValue = color & 0xFFFF) & 1) { \
                            red = ((colorValue >> 11) & 0x1F) & 0xFFFF;
                            green = (colorValue >> 6) & 0x1F;
                            colorValue = (blue = (colorValue >> 1) & 0x1F);
                            scaleValue = paletteScale;
                            red = red * scaleValue;
                            scaledRed = red / 0x100;
                            red = scaledRed;
                            green = (green * paletteScale) / 0x100;
                            colorValue = green;
                            blue = (blue * paletteScale) / 0x100;
                            *dst = (red << 11) | (colorValue << 6) | (blue << 1) | 1;
                        }
                        dst += 2;
                        dst--;
                        if (i != sizeof(MenuPalette)) {
                            goto palette_loop;
                        }

                        gDPLoadTextureTile_4b(
                            gRegionAllocPtr++, texture->imageOffset + (u8*) font, G_IM_FMT_CI, texture->width,
                            texture->height, 0,
                            0, // @recomp Exclude the extra row and column from the inclusive load bounds.
                            texture->width - 1, texture->height - 1, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK,
                            G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, scratch);
                        gSPTextureRectangle(gRegionAllocPtr++, x0 << 2, storedY << 2, x1 << 2, y1 << 2, G_TX_RENDERTILE,
                                            clipS << 5, clipT << 5, 0x400, 0x400);
                    }
                }
            }
        }
    }
}

RECOMP_PATCH void drawMenuPanelBackdrop(s32 x, s32 y, s32 width, s32 height) {

    Gfx *volatile unused0;
    Gfx *volatile unused1;
    Gfx *volatile unused2;
    register s32 widthU;
    register s32 heightU;
    s32 ulx;
    s32 uly;

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureLUT(gRegionAllocPtr++, G_TT_NONE);
    gDPSetCombineMode(gRegionAllocPtr++, G_CC_MODULATEI_PRIM, G_CC_MODULATEI_PRIM);
    gDPSetRenderMode(gRegionAllocPtr++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        gMenuPanelBackdropTexture,
        G_IM_FMT_I,
        16,
        8,
        0,
        0,
        // @recomp Exclude the extra row and column from the inclusive load bounds.
        16 - 1,
        8 - 1,
        0,
        G_TX_NOMIRROR | G_TX_CLAMP,
        G_TX_NOMIRROR | G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0, 0, 0, 0x64);


    widthU = (u16)width;
    widthU = (u16)width;
    heightU = (u16)height;
    ulx = ((s16)x + gMenuViewportCenterX) << 2;
    uly = ((s16)y + gMenuViewportCenterY) << 2;

    gSPTextureRectangle(
        gRegionAllocPtr++,
        ulx,
        uly,
        ulx + (((widthU << 4) << 2) / 0x1000),
        (((heightU << 3) << 2) / 0x1000) + uly,
        G_TX_RENDERTILE,
        0,
        0,
        (0x400000 / widthU) & 0xFFFFU,
        (0x400000 / (u16)height) & 0xFFFF
    );
    gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
    { Gfx *volatile unused3; }
}
