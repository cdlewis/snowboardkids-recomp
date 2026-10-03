#include "patches.h"
#include "race_split_screen.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/viewport_manager.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/race_state.h"
#include "game/race/ui/race_hud.h"

#define RACE_HUD_MAIN_FONT_HANDLE (gAssetHandles[0x1C])
#define RACE_HUD_POPUP_FONT_HANDLE (gAssetHandles[0x1F])

extern u8 gCurrentViewportIndex;
extern const char gRaceHudTwoPlayerScoreFormat[];
extern s16 gRaceHudCoinSpinnerFrame;
extern s16 gRaceLapCount;
extern Gfx *gRegionAllocPtr;
extern f32 recomp_get_target_hud_aspect_ratio(f32 original);

void anchorVerticalHudGroup(s32 side) {
    ViewportState *viewport = &gViewportStates[gCurrentViewportIndex];
    f32 targetAspect = recomp_get_target_aspect_ratio(4.0f / 3.0f);
    f32 hudAspect = recomp_get_target_hud_aspect_ratio(4.0f / 3.0f);
    f32 height = viewport->bottom - viewport->top;
    f32 viewportAspect = targetAspect * ((viewport->right - viewport->left) / 320.0f) / (height / 240.0f);
    f32 safeAspect = hudAspect < viewportAspect ? hudAspect : viewportAspect;
    f32 aspectScale = targetAspect / (4.0f / 3.0f);
    f32 center = (viewport->left + viewport->right) * 0.5f;
    // Match SBK2's viewport-local safe width, leaving sprites at their native proportions.
    f32 shift = (center - 160.0f) * (aspectScale - 1.0f) + side * (height * safeAspect * 0.5f - 80.0f);
    s32 offset = (s32)(shift * 4.0f + (shift < 0.0f ? -0.5f : 0.5f));

    // Explicit centre alignment enables RT64 texture-grid correction, preventing repeated texture-edge columns.
    s32 origin = side != 0 ? G_EX_ORIGIN_CENTER : G_EX_ORIGIN_NONE;
    // Compensate for the native centre added by the explicit origin without changing HUD placement.
    if (side != 0) {
        offset -= 160 * 4;
    }

    gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_ADJUST);
    gEXSetRectAlign(gRegionAllocPtr++, origin, origin, offset, 0, offset, 0);
}

RECOMP_PATCH void drawTwoPlayerRaceHud(void *arg0) {
    s32 y;
    s32 x;
    s32 color;
    char *ptr;
    char buffer[32];
    // @recomp Select corner placement only for the vertical two-player race layout.
    s32 vertical = raceUsesVerticalTwoPlayerSplit();

    if (gCurrentViewportIndex == 0) {
        y = -0x28;
    } else {
        y = 0x1A;
    }

    // @recomp Move the vertical score group to the bottom-right corner.
    if (vertical) {
        // @recomp Keep the 16-pixel coin and score eight pixels above the viewport bottom.
        y = 96;
        // @recomp Anchor the score, coin and following item boxes to this viewport's right HUD edge.
        anchorVerticalHudGroup(1);
    }

    // @recomp Use the game formatter wrapper because patch calls to sprintf do not resolve.
    _Sprintf(buffer, gRaceHudTwoPlayerScoreFormat, gRacePlayers[gCurrentViewportIndex].score);
    // @recomp Keep the five score digits immediately left of the coin at the vertical right inset.
    x = vertical ? 16 : 0x50;
    ptr = buffer;
    if (gRacePlayers[gCurrentViewportIndex].score < 0x64) {
        color = 0x10;
    } else {
        color = 0xE;
    }

    do {
        if (*ptr != ' ') {
            drawAssetTableSpriteWithExplicitPalette(
                x,
                y,
                getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
                *ptr - 5,
                color
            );
        }
        ptr++;
        x += 8;
        // @recomp Draw the original five score characters without depending on an adjacent stack variable.
    } while (ptr != buffer + 5);

    drawAssetTableSprite(
        // @recomp Place the 16-pixel coin eight pixels inside the vertical right HUD edge.
        vertical ? 56 : 0x78,
        y,
        getRelocatableHeapBlockBase(RACE_HUD_MAIN_FONT_HANDLE),
        (gRaceHudCoinSpinnerFrame >> 1) + 4
    );

    if (gRacePlayers[gCurrentViewportIndex].itemEffectPalette != 0) {
        drawScaledAssetTableSprite(
            // @recomp Place the first item box beside the second at the vertical right HUD edge.
            vertical ? 8 : -0x88,
            // @recomp Place the vertical item box eight pixels below the viewport top.
            vertical ? -112 : -0x30,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceHudItemEffectTileOffsets[gRacePlayers[gCurrentViewportIndex].itemEffectType] +
                gRacePlayers[gCurrentViewportIndex].itemEffectCount - 1,
            gRacePlayers[gCurrentViewportIndex].itemEffectPalette
        );
    } else {
        drawAssetTableSprite(
            // @recomp Place the first item box beside the second at the vertical right HUD edge.
            vertical ? 8 : -0x88,
            // @recomp Place the vertical item box eight pixels below the viewport top.
            vertical ? -112 : -0x30,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceHudItemEffectTileOffsets[gRacePlayers[gCurrentViewportIndex].itemEffectType] +
                gRacePlayers[gCurrentViewportIndex].itemEffectCount - 1
        );
    }

    if (gRacePlayers[gCurrentViewportIndex].actionEffectPalette != 0) {
        drawScaledAssetTableSprite(
            // @recomp Keep the second 32-pixel item box eight pixels inside the vertical right HUD edge.
            vertical ? 40 : -0x68,
            // @recomp Place the vertical item box eight pixels below the viewport top.
            vertical ? -112 : -0x30,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceHudActionEffectTileIds[gRacePlayers[gCurrentViewportIndex].actionEffectType],
            gRacePlayers[gCurrentViewportIndex].actionEffectPalette
        );
    } else {
        drawAssetTableSprite(
            // @recomp Keep the second 32-pixel item box eight pixels inside the vertical right HUD edge.
            vertical ? 40 : -0x68,
            // @recomp Place the vertical item box eight pixels below the viewport top.
            vertical ? -112 : -0x30,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceHudActionEffectTileIds[gRacePlayers[gCurrentViewportIndex].actionEffectType]
        );
    }

    // @recomp Switch the vertical rank and lap groups to this viewport's left HUD edge.
    if (vertical) {
        // @recomp Apply the left anchor before drawing the rank and lap label.
        anchorVerticalHudGroup(-1);
    }
    drawAssetTableSprite(
        // @recomp Align the vertical rank sprite with the lap label at the eight-pixel left inset.
        vertical ? -72 : -0x88,
        // @recomp Keep the 32-pixel vertical rank sprite eight pixels above the viewport bottom.
        vertical ? 80 : 0x12,
        getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
        gRacePlayers[gCurrentViewportIndex].rankIndex
    );

    if (gCurrentViewportIndex == 0) {
        y = -0x30;
    } else {
        y = 0x2A;
    }
    // @recomp Move the vertical lap display to the top-left corner.
    if (vertical) {
        // @recomp Match the item boxes with an eight-pixel top inset.
        y = -112;
    }
    // @recomp Place the vertical lap label at the same eight-pixel left inset as the rank.
    drawAssetTableSprite(vertical ? -72 : 0x58, y, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x1A);
    // @recomp Restore centred alignment so later overlay draws do not inherit the left HUD anchor.
    if (vertical) {
        // @recomp Reset the vertical viewport's rectangle origin and offset.
        anchorVerticalHudGroup(0);
    }
}

RECOMP_PATCH void drawTwoPlayerLapCounter(void *arg0) {
    s32 y;
    s32 viewportIndex;

    // @recomp Select corner placement only for the vertical two-player race layout.
    s32 vertical = raceUsesVerticalTwoPlayerSplit();

    viewportIndex = gCurrentViewportIndex;
    if (viewportIndex == 0) {
        y = -0x30;
    } else {
        y = 0x2A;
    }

    // @recomp Move the vertical lap display to the top-left corner.
    if (vertical) {
        // @recomp Match the item boxes with an eight-pixel top inset.
        y = -112;
        // @recomp Use the same left HUD anchor as the lap label.
        anchorVerticalHudGroup(-1);
    }
    // @recomp Position the vertical current lap digit and use the exported implementation of the weak ASCII alias.
    drawMenuAsciiCharImpl(vertical ? -48 : 0x70, (s16)y, gRacePlayers[viewportIndex].lapDigit + '1', 2);
    // @recomp Position the vertical lap separator and use the exported implementation of the weak ASCII alias.
    drawMenuAsciiCharImpl(vertical ? -40 : 0x78, (s16)y, '/', 2);
    // @recomp Position the vertical total lap digit and use the exported implementation of the weak ASCII alias.
    drawMenuAsciiCharImpl(vertical ? -32 : 0x80, (s16)y, gRaceLapCount + '0', 2);
    // @recomp Restore centred alignment so later overlay draws do not inherit the left HUD anchor.
    if (vertical) {
        // @recomp Reset the vertical viewport's rectangle origin and offset.
        anchorVerticalHudGroup(0);
    }
}
