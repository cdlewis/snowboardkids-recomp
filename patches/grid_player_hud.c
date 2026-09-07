#include "patches.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/viewport_manager.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/race_state.h"
#include "game/race/ui/race_hud.h"

#define RACE_HUD_MAIN_FONT_HANDLE (gAssetHandles[0x1C])
#define RACE_HUD_POPUP_FONT_HANDLE (gAssetHandles[0x1F])

extern Gfx *gRegionAllocPtr;
extern u8 gCurrentViewportIndex;
extern s16 gRaceHudCoinSpinnerFrame;
extern s16 gRaceLapCount;
extern u8 gRaceTimerTensDigitTileOffsets[8];
extern u8 gRaceTimerOnesDigitTileIds[8];
extern const char gRaceHudMultiplayerScoreFormat[];

static void anchorGridHudGroup(s32 side) {
    ViewportState *viewport = &gViewportStates[gCurrentViewportIndex];
    f32 scale = recomp_get_target_aspect_ratio(4.0f / 3.0f) / (4.0f / 3.0f);
    f32 center = (viewport->left + viewport->right) * 0.5f;
    f32 halfWidth = (viewport->right - viewport->left) * 0.5f;
    // SBK2 uses the complete physical quadrant for 3/4P, without another global HUD-ratio clamp.
    f32 shift = (center - 160.0f + side * halfWidth) * (scale - 1.0f);
    s32 offset = (s32)(shift * 4.0f + (shift < 0.0f ? -0.5f : 0.5f));
    gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_ADJUST);
    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, offset, 0, offset, 0);
}

RECOMP_PATCH void drawMultiplayerRaceHud(void *arg0) {
    RacePlayer *player;
    AssetTable *texture;

    anchorGridHudGroup(1);

    drawScaledAssetTableSprite(
        0x38,
        0x24,
        getRelocatableHeapBlockBase(RACE_HUD_MAIN_FONT_HANDLE),
        (gRaceHudCoinSpinnerFrame >> 1) + 4,
        1
    );

    texture = getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE);
    player = &gRacePlayers[gCurrentViewportIndex];
    drawScaledAssetTableSprite(
        0x20,
        -0x38,
        texture,
        gRaceTimerTensDigitTileOffsets[gRacePlayers[gCurrentViewportIndex].itemEffectType] +
            gRacePlayers[gCurrentViewportIndex].itemEffectCount - 1,
        player->itemEffectPalette + 1
    );

    texture = getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE);
    player = &gRacePlayers[gCurrentViewportIndex];
    drawScaledAssetTableSprite(
        0x30,
        -0x38,
        texture,
        gRaceTimerOnesDigitTileIds[player->actionEffectType],
        player->actionEffectPalette + 1
    );

    texture = getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE);
    anchorGridHudGroup(-1);
    // Half-size 32px sprites are recentered by +8px by the original sprite helper.
    // Account for that inset when anchoring the visible rank and item rectangles.
    drawScaledAssetTableSprite(-0x50, 0x18, texture, gRacePlayers[gCurrentViewportIndex].rankIndex, 1);

    drawAssetTableSprite(-0x48, -0x30, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x1A);
    anchorGridHudGroup(0);
}

RECOMP_PATCH void drawMultiplayerScoreAndLapCounter(void *arg0) {
    s32 x;
    s32 palette;
    char buffer[0x20];

    anchorGridHudGroup(1);
    _Sprintf(buffer, gRaceHudMultiplayerScoreFormat, gRacePlayers[gCurrentViewportIndex].score);
    if (gRacePlayers[gCurrentViewportIndex].score < 0x64) {
        palette = 1;
    } else {
        palette = 2;
    }
    drawMenuAsciiTextDefaultScale(0x14, 0x28, buffer, palette);

    anchorGridHudGroup(-1);
    x = -0x30;

    drawMenuAsciiCharImpl((s16)x, -0x30, gRacePlayers[gCurrentViewportIndex].lapDigit + '1', 2);
    drawMenuAsciiCharImpl((s16)(x + 8), -0x30, 0x2F, 2);
    drawMenuAsciiCharImpl((s16)(x + 0x10), -0x30, gRaceLapCount + 0x30, 2);
    anchorGridHudGroup(0);
}
