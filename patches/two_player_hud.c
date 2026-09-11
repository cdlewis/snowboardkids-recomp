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

    gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_ADJUST);
    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, offset, 0, offset, 0);
}

// Keep the original row layout; vertical races place the same groups in their viewport's four corners.
RECOMP_PATCH void drawTwoPlayerRaceHud(void *arg0) {
    s32 y;
    s32 x;
    s32 color;
    char *ptr;
    char buffer[32];
    s32 vertical = raceUsesVerticalTwoPlayerSplit();

    if (gCurrentViewportIndex == 0) {
        y = -0x28;
    } else {
        y = 0x1A;
    }

    if (vertical) {
        y = 88;
        anchorVerticalHudGroup(1);
    }

    _Sprintf(buffer, gRaceHudTwoPlayerScoreFormat, gRacePlayers[gCurrentViewportIndex].score);
    x = vertical ? 8 : 0x50;
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
    } while (ptr != buffer + 5);

    drawAssetTableSprite(
        vertical ? 48 : 0x78,
        y,
        getRelocatableHeapBlockBase(RACE_HUD_MAIN_FONT_HANDLE),
        (gRaceHudCoinSpinnerFrame >> 1) + 4
    );

    if (gRacePlayers[gCurrentViewportIndex].itemEffectPalette != 0) {
        drawScaledAssetTableSprite(
            vertical ? 8 : -0x88,
            vertical ? -96 : -0x30,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceHudItemEffectTileOffsets[gRacePlayers[gCurrentViewportIndex].itemEffectType] +
                gRacePlayers[gCurrentViewportIndex].itemEffectCount - 1,
            gRacePlayers[gCurrentViewportIndex].itemEffectPalette
        );
    } else {
        drawAssetTableSprite(
            vertical ? 8 : -0x88,
            vertical ? -96 : -0x30,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceHudItemEffectTileOffsets[gRacePlayers[gCurrentViewportIndex].itemEffectType] +
                gRacePlayers[gCurrentViewportIndex].itemEffectCount - 1
        );
    }

    if (gRacePlayers[gCurrentViewportIndex].actionEffectPalette != 0) {
        drawScaledAssetTableSprite(
            vertical ? 40 : -0x68,
            vertical ? -96 : -0x30,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceHudActionEffectTileIds[gRacePlayers[gCurrentViewportIndex].actionEffectType],
            gRacePlayers[gCurrentViewportIndex].actionEffectPalette
        );
    } else {
        drawAssetTableSprite(
            vertical ? 40 : -0x68,
            vertical ? -96 : -0x30,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceHudActionEffectTileIds[gRacePlayers[gCurrentViewportIndex].actionEffectType]
        );
    }

    if (vertical) {
        anchorVerticalHudGroup(-1);
    }
    drawAssetTableSprite(
        vertical ? -68 : -0x88,
        vertical ? 80 : 0x12,
        getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
        gRacePlayers[gCurrentViewportIndex].rankIndex
    );

    if (gCurrentViewportIndex == 0) {
        y = -0x30;
    } else {
        y = 0x2A;
    }
    if (vertical) {
        y = -96;
    }
    drawAssetTableSprite(vertical ? -68 : 0x58, y, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x1A);
    if (vertical) {
        anchorVerticalHudGroup(0);
    }
}

RECOMP_PATCH void drawTwoPlayerLapCounter(void *arg0) {
    s32 y;
    s32 viewportIndex;

    s32 vertical = raceUsesVerticalTwoPlayerSplit();

    viewportIndex = gCurrentViewportIndex;
    if (viewportIndex == 0) {
        y = -0x30;
    } else {
        y = 0x2A;
    }

    if (vertical) {
        y = -96;
        anchorVerticalHudGroup(-1);
    }
    drawMenuAsciiCharImpl(vertical ? -44 : 0x70, (s16)y, gRacePlayers[viewportIndex].lapDigit + '1', 2);
    drawMenuAsciiCharImpl(vertical ? -36 : 0x78, (s16)y, '/', 2);
    drawMenuAsciiCharImpl(vertical ? -28 : 0x80, (s16)y, gRaceLapCount + '0', 2);
    if (vertical) {
        anchorVerticalHudGroup(0);
    }
}
