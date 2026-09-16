#include "patches.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "rt64_extended_gbi.h"

extern Gfx *gRegionAllocPtr;
extern s16 gMenuViewportWidth;
extern s16 gMenuViewportHeight;
extern s16 gMenuViewportCenterX;
extern s16 gMenuViewportCenterY;
extern s16 gMenuSpriteFlipScales[8];

static s32 sDrawingMenuBackdrop;

RECOMP_PATCH void drawScrollingMenuBackdropActor(void *arg0) {
    SpriteActor *actor = arg0;

    // @recomp Keep background tiles proportional while stretching their scissor across the output.
    sDrawingMenuBackdrop = 1;
    gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_ADJUST);
    gEXSetScissorAspect(gRegionAllocPtr++, G_EX_ASPECT_STRETCH);
    drawMenuTilemapSprite(&actor->sprite.render, MENU_TILEMAP_TEXEL_4B, actor->x, actor->y);
    // @recomp Restore automatic clipping and scaling before the remaining menu callbacks.
    gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_AUTO);
    gEXSetScissorAspect(gRegionAllocPtr++, G_EX_ASPECT_AUTO);
    sDrawingMenuBackdrop = 0;
}

RECOMP_PATCH void drawMenuTilemapSprite(
    MenuRenderSprite *sprite,
    MenuTilemapTexelSize texelSize,
    s16 tilemapWidth,
    s16 tilemapHeight
) {
    s16 sourceX;
    s16 sourceY;
    s16 drawX;
    s16 drawY;
    s16 clipLeft;
    s16 clipTop;
    s16 clipRight;
    s16 clipBottom;
    u16 tileMask;
    s16 firstDrawX;
    s16 columnCount;
    s16 rowCount;
    s16 column;
    s16 row;
    s16 scaleS;
    s16 scaleT;
    s16 rectLeft;
    s16 rectTop;
    s16 rectRight;
    s16 rectBottom;
    s16 tileId;
    s16 texS;
    s16 texT;
    s16 imageIndex;
    s16 tileX;
    s16 tileY;
    u16 tileShift;
    s16 *tilemap;
    u16 *images;
    s32 sourceOffsetX;
    MenuRenderSprite *render;
    u32 paletteIndex;
    MenuTilemapTile *tiles;
    u8 *paletteData;

    tilemap = (render = sprite)->tilemap;
    images = render->images;
    tiles = render->tiles;
    paletteData = render->paletteData;

    clipLeft = render->viewportX;
    sourceX = render->scrollX;
    if (clipLeft < -gMenuViewportWidth / 2) {
        clipLeft = -gMenuViewportWidth / 2;
        sourceX = (sourceX + clipLeft) - render->viewportX;
    }

    clipTop = render->viewportY;
    sourceY = render->scrollY;
    if (clipTop < -gMenuViewportHeight / 2) {
        clipTop = -gMenuViewportHeight / 2;
        sourceY = (sourceY + clipTop) - render->viewportY;
    }

    clipRight = render->viewportX + render->viewportWidth;
    if (clipRight > gMenuViewportWidth / 2) {
        clipRight = gMenuViewportWidth / 2;
    }

    clipBottom = render->viewportY + render->viewportHeight;
    if (clipBottom > gMenuViewportHeight / 2) {
        clipBottom = gMenuViewportHeight / 2;
    }

    // @recomp Extend only the scrolling menu backdrop, preserving its tile scale and scroll phase.
    if (sDrawingMenuBackdrop) {
        s32 halfWidth = (s32)(recomp_get_target_aspect_ratio(4.0f / 3.0f) * 120.0f + 0.999f);
        s32 repeatWidth = tilemapWidth * render->tileWidth;
        s32 repeatHeight = tilemapHeight * render->tileHeight;
        clipLeft = -halfWidth;
        clipRight = halfWidth;
        clipTop = -120;
        clipBottom = 120;
        // @recomp Wrap the added left and top tiles to positive indices before the original tile lookup.
        sourceX = ((render->scrollX + clipLeft - render->viewportX) % repeatWidth + repeatWidth) % repeatWidth;
        sourceY = ((render->scrollY + clipTop - render->viewportY) % repeatHeight + repeatHeight) % repeatHeight;
    }

    clipLeft += gMenuViewportCenterX;
    clipRight += gMenuViewportCenterX;
    clipTop += gMenuViewportCenterY;
    clipBottom += gMenuViewportCenterY;

    tileMask = render->tileWidth - 1;
    if (render->tileWidth == 0x10) {
        tileShift = 4;
    } else {
        tileShift = 5;
    }

    sourceOffsetX = sourceX & tileMask;
    columnCount = ((clipRight - clipLeft) + tileMask - 1) >> tileShift;
    rowCount = ((clipBottom - clipTop) + tileMask - 1) >> tileShift;

    if (sourceX & tileMask) {
        columnCount++;
    }
    firstDrawX = clipLeft - sourceOffsetX;

    if (sourceY & tileMask) {
        rowCount++;
    }
    drawY = clipTop - (sourceY & tileMask);
    tileY = (sourceY >> tileShift) % tilemapHeight;

    for (row = 0; row < rowCount; row++) {
        drawX = firstDrawX;
        tileX = (sourceX >> tileShift) % tilemapWidth;

        for (column = 0; column < columnCount; column++) {
            imageIndex = tileX + (tileY * render->tilemapWidth);
            tileId = tilemap[imageIndex];
            paletteIndex = tiles[tileId].paletteIndex;
            imageIndex = tiles[tileId].imageIndex;

            if (tileId != 0) {
                scaleS = gMenuSpriteFlipScales[tiles[tileId].flip * 2];
                scaleT = gMenuSpriteFlipScales[tiles[tileId].flip * 2 + 1];
                rectLeft = drawX;
                rectTop = drawY;
                rectRight = drawX + render->tileWidth;
                rectBottom = drawY + render->tileHeight;
                texS = 0;
                texT = 0;

                if (scaleS == -1) {
                    texS = render->tileWidth - 1;
                }
                if (scaleT == -1) {
                    texT = render->tileHeight - 1;
                }

                if ((drawX < clipRight) && (drawY < clipBottom) && (rectRight >= clipLeft) &&
                    (rectBottom >= clipTop)) {
                    if (drawX < clipLeft) {
                        texS = clipLeft - drawX;
                        if (scaleS == -1) {
                            texS = (render->tileWidth - texS) - 1;
                        }
                        rectLeft = clipLeft;
                    }

                    if (drawY < clipTop) {
                        texT = clipTop - drawY;
                        if (scaleT == -1) {
                            texT = (render->tileHeight - texT) - 1;
                        }
                        rectTop = clipTop;
                    }

                    if (rectRight >= clipRight) {
                        // @recomp Cover the final backdrop pixel with the one-cycle rectangle's exclusive endpoint.
                        rectRight = sDrawingMenuBackdrop ? clipRight : clipRight - 1;
                    }
                    if (rectBottom >= clipBottom) {
                        // @recomp Cover the bottom backdrop pixel with the one-cycle rectangle's exclusive endpoint.
                        rectBottom = sDrawingMenuBackdrop ? clipBottom : clipBottom - 1;
                    }

                    if ((u8)texelSize == MENU_TILEMAP_TEXEL_4B) {
                        gDPLoadTLUT_pal16(gRegionAllocPtr++, paletteIndex, paletteData + (paletteIndex << 5));
                        gDPLoadTextureTile_4b(
                            gRegionAllocPtr++,
                            &images[((imageIndex - 1) * render->tileWidth * render->tileHeight) / 4],
                            G_IM_FMT_CI,
                            render->tileWidth,
                            render->tileHeight,
                            0,
                            0,
                            render->tileWidth,
                            render->tileHeight,
                            paletteIndex,
                            G_TX_CLAMP,
                            G_TX_CLAMP,
                            G_TX_NOMASK,
                            G_TX_NOMASK,
                            G_TX_NOLOD,
                            G_TX_NOLOD
                        );
                    } else {
                        if (tilemapHeight) {
                        }
                        gDPLoadTLUT_pal256(gRegionAllocPtr++, paletteData + (paletteIndex << 5));
                        gDPLoadTextureTile(
                            gRegionAllocPtr++,
                            &images[((imageIndex - 1) * render->tileWidth * render->tileHeight) / 2],
                            G_IM_FMT_CI,
                            G_IM_SIZ_8b,
                            render->tileWidth,
                            render->tileHeight,
                            0,
                            0,
                            render->tileWidth,
                            render->tileHeight,
                            0,
                            G_TX_CLAMP,
                            G_TX_CLAMP,
                            G_TX_NOMASK,
                            G_TX_NOMASK,
                            G_TX_NOLOD,
                            G_TX_NOLOD
                        );
                    }

                    // @recomp Use signed coordinates only for backdrop tiles beyond the native screen edges.
                    if (sDrawingMenuBackdrop) {
                        // @recomp Overlap the next tile by one pixel so RT64's scissor-edge correction cannot open a gap.
                        // @recomp Keep the texture step unchanged; the next column draws over the extra edge texel.
                        gEXTextureRectangle(
                            gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE,
                            rectLeft * 4,
                            rectTop * 4,
                            (rectRight + 1) * 4,
                            rectBottom * 4,
                            G_TX_RENDERTILE,
                            texS << 5,
                            texT << 5,
                            (u16)(scaleS << 10),
                            (u16)(scaleT << 10)
                        );
                    } else {
                        gSPTextureRectangle(
                            gRegionAllocPtr++,
                            rectLeft << 2,
                            rectTop << 2,
                            rectRight << 2,
                            rectBottom << 2,
                            G_TX_RENDERTILE,
                            texS << 5,
                            texT << 5,
                            (u16)(scaleS << 10),
                            (u16)(scaleT << 10)
                        );
                    }
                }
            }

            drawX += render->tileWidth;
            tileX = (tileX + 1) % tilemapWidth;
        }

        drawY += render->tileHeight;
        tileY = (tileY + 1) % tilemapHeight;
    }
}
