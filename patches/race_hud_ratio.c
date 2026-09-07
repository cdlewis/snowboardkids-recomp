#include "patches.h"
#include "race_split_screen.h"

#include "game/race/race_state.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/race/ui/race_hud.h"
#include "game/race/player/race_player_input.h"

#define RACE_HUD_MAIN_FONT_HANDLE (gAssetHandles[0x1C])
#define RACE_HUD_POPUP_FONT_HANDLE (gAssetHandles[0x1F])

#define HUD_SCREEN_WIDTH 320
#define HUD_SCREEN_HEIGHT 240

int _Sprintf(char *buffer, const char *fmt, ...);

extern Gfx *gRegionAllocPtr;

extern const char gRaceHudSinglePlayerScoreFormat[];
extern s16 gRaceHudCoinSpinnerFrame;
extern s16 gRaceHudMode;
extern u8 gRaceTimerTensDigitTileOffsets[8];
extern u8 gRaceTimerOnesDigitTileIds[8];
extern s16 gRaceLapCount;
extern u16 gRaceProgressMeterIconTiles[36];
extern u16 gRaceProgressMeterIconPalettes[6];

#define HUD_ORIGIN_SHIFT(origin) \
    (((s32) (origin) * HUD_SCREEN_WIDTH * 4) / G_EX_ORIGIN_RIGHT)

static void hudAnchor(u32 origin) {
    s32 offset = -HUD_ORIGIN_SHIFT(origin);

    gEXSetRectAlign(gRegionAllocPtr++, origin, origin, offset, 0, offset, 0);
}

static void hudBeginAnchoredDraw(void) {
    gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_LEFT, G_EX_ORIGIN_RIGHT, 0, 0,
                       -((s32) HUD_SCREEN_WIDTH), 0, 0, 0, (s32) HUD_SCREEN_WIDTH,
                       HUD_SCREEN_HEIGHT);
    gDPSetScissor(gRegionAllocPtr++, G_SC_NON_INTERLACE, 0, 0, (s32) HUD_SCREEN_WIDTH,
                  HUD_SCREEN_HEIGHT);
}

static void hudEndAnchoredDraw(void) {
    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
    gEXSetScissorAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0, 0, 0,
                       (s32) HUD_SCREEN_WIDTH, HUD_SCREEN_HEIGHT);
    gDPSetScissor(gRegionAllocPtr++, G_SC_NON_INTERLACE, 0, 0, (s32) HUD_SCREEN_WIDTH,
                  HUD_SCREEN_HEIGHT);
}

RECOMP_PATCH void drawSinglePlayerRaceHud(void *arg0) {
    s32 palette;
    s32 i;
    s32 var_s1;
    char buffer[0x20];

    _Sprintf(buffer, gRaceHudSinglePlayerScoreFormat, gRacePlayers[0].score);
    if (gRacePlayers[0].score < 100) {
        palette = 0x10;
    } else {
        palette = 0xE;
    }

    // @recomp Allow RT64 to move HUD groups beyond the original 4:3 scissor.
    hudBeginAnchoredDraw();

    // @recomp Keep the gold counter and spinner against the right safe-area edge.
    hudAnchor(G_EX_ORIGIN_RIGHT);

    for (i = 0, var_s1 = 0x50; i < 5; i++, var_s1 += 8) {
        if (buffer[i] != ' ') {
            drawAssetTableSpriteWithExplicitPalette(
                var_s1,
                0x50,
                getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
                (u8)buffer[i] - 5,
                palette
            );
        }
    }

    drawAssetTableSprite(
        0x78,
        0x50,
        getRelocatableHeapBlockBase(RACE_HUD_MAIN_FONT_HANDLE),
        (gRaceHudCoinSpinnerFrame >> 1) + 4
    );

    // @recomp Keep the held-item and action-effect sprites around the safe-area centre.
    hudAnchor(G_EX_ORIGIN_CENTER);

    if (gRacePlayers[0].itemEffectPalette != 0) {
        drawScaledAssetTableSprite(
            -0x20,
            -0x60,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceTimerTensDigitTileOffsets[gRacePlayers[0].itemEffectType] + gRacePlayers[0].itemEffectCount - 1,
            gRacePlayers[0].itemEffectPalette
        );
    } else {
        drawAssetTableSprite(
            -0x20,
            -0x60,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceTimerTensDigitTileOffsets[gRacePlayers[0].itemEffectType] + gRacePlayers[0].itemEffectCount - 1
        );
    }

    if (gRacePlayers[0].actionEffectPalette != 0) {
        drawScaledAssetTableSprite(
            0,
            -0x60,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceTimerOnesDigitTileIds[gRacePlayers[0].actionEffectType],
            gRacePlayers[0].actionEffectPalette
        );
    } else {
        drawAssetTableSprite(
            0,
            -0x60,
            getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
            gRaceTimerOnesDigitTileIds[gRacePlayers[0].actionEffectType]
        );
    }

    // @recomp Keep the race position and lap group against the left safe-area edge.
    hudAnchor(G_EX_ORIGIN_LEFT);

    drawAssetTableSprite(
        -0x88,
        0x40,
        getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
        gRacePlayers[0].rankIndex
    );
    drawAssetTableSprite(-0x88, -0x60, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x39);
    drawAssetTableSpriteWithExplicitPalette(
        -0x68,
        -0x60,
        getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
        gRacePlayers[0].lapDigit + 0x2C,
        0xE
    );
    drawAssetTableSprite(-0x5C, -0x60, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x38);
    drawAssetTableSpriteWithExplicitPalette(
        -0x50,
        -0x60,
        getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
        gRaceLapCount + 0x2B,
        0xE
    );

    // @recomp Restore the original rectangle state after the single-player HUD.
    hudEndAnchoredDraw();
}

RECOMP_PATCH void drawThreePlayerHudDivider(void *arg0) {
    f32 scale = recomp_get_target_aspect_ratio(4.0f / 3.0f) / (4.0f / 3.0f);
    s32 offset = (s32)((HUD_SCREEN_WIDTH / 4) * (scale - 1.0f) * 4.0f + 0.5f);

    // The NO ENTRY panel belongs to the unused bottom-right quadrant, not the full-screen centre.
    gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_ADJUST);
    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, offset, 0, offset, 0);
    drawAssetTableSprite(0xC, 0x2C, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x90);
    gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
}

RECOMP_PATCH void drawRaceCourseProgressMeter(void *arg0) {
    s32 i;
    s32 j;
    union {
        s32 playerId;
        AssetTable *assetTable;
    } temp;
    s32 order[4];
    s16 xBase;
    s16 yBase;

    order[0] = 0;
    order[1] = 1;
    order[2] = 2;
    order[3] = 3;

    for (i = 0; i < 3; i++) {
        for (j = i + 1; j < 4; j++) {
            if (gRacePlayers[order[j]].rankIndex < gRacePlayers[order[i]].rankIndex) {
                temp.playerId = order[i];
                order[i] = order[j];
                order[j] = temp.playerId;
            }
        }
    }

    if (gRaceHudMode == RACE_HUD_MODE_ONE_PLAYER) {
        xBase = 0x78;
        yBase = -0x56;
    }
    if (gRaceHudMode == RACE_HUD_MODE_TWO_PLAYER) {
        xBase = raceUsesVerticalTwoPlayerSplit() ? -8 : 0x78;
        yBase = -0x48;
    }
    if ((gRaceHudMode == RACE_HUD_MODE_THREE_PLAYER) || (gRaceHudMode == RACE_HUD_MODE_FOUR_PLAYER)) {
        xBase = -8;
        yBase = -0x48;
    }

    // @recomp Expand the scissor and anchor the meter to the edge used by the current layout.
    hudBeginAnchoredDraw();

    hudAnchor(xBase == 0x78 ? G_EX_ORIGIN_RIGHT : G_EX_ORIGIN_CENTER);
    if (xBase == -8) {
        AssetTable *table = getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE);
        // Match SBK2's covered-edge correction: centre the visible stroke, not the sprite bounds.
        s32 center = (HUD_SCREEN_WIDTH / 2 + xBase + 4) * 4 + table->entries[0x50].width * 2 - 4;
        gEXSetRectAspect(gRegionAllocPtr++, G_EX_ASPECT_ADJUST);
        gEXSetRectAlign(gRegionAllocPtr++, G_EX_ORIGIN_CENTER, G_EX_ORIGIN_CENTER, -center, 0, -center, 0);
    }
    drawAssetTableSprite(
        (s16)(xBase + 4),
        (s16)(yBase + 4),
        getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
        0x50
    );

    i = 3;
    do {
        if (((gRacePlayers[order[i]].stateFlags & RACE_PLAYER_COLLISION_SQUASHED) != 0) || (gRacePlayers[order[i]].progressMeterSquashFrame != 0)) {
            gRacePlayers[order[i]].progressMeterSquashFrame++;
        }
        if (((gRacePlayers[order[i]].stateFlags & RACE_PLAYER_COLLISION_SQUASHED) != 0) && (gRacePlayers[order[i]].progressMeterSquashFrame >= 5)) {
            gRacePlayers[order[i]].progressMeterSquashFrame = 4;
        }
        if (gRacePlayers[order[i]].progressMeterSquashFrame >= 6) {
            gRacePlayers[order[i]].progressMeterSquashFrame = 0;
        }

        if (gRacePlayers[order[i]].progressMeterSquashFrame != 0) {
            if (gRacePlayers[order[i]].activeSparkleEffectCount != 0) {
                temp.assetTable = getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE);
                drawAssetTableSpriteWithExplicitPalette(
                    (s16)(xBase - 8),
                    (s16)(gRacePlayers[order[i]].progressMeterPosition + yBase),
                    temp.assetTable,
                    (
                        &gRaceProgressMeterIconTiles[gRacePlayers[order[i]].progressMeterSquashFrame]
                    )[gRacePlayers[order[i]].characterId * 6],
                    gRaceProgressMeterIconPalettes[gRacePlayers[order[i]].characterId]
                );
            } else {
                temp.assetTable = getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE);
                drawAssetTableSprite(
                    (s16)(xBase - 8),
                    (s16)(gRacePlayers[order[i]].progressMeterPosition + yBase),
                    temp.assetTable,
                    (
                        &gRaceProgressMeterIconTiles[gRacePlayers[order[i]].progressMeterSquashFrame]
                    )[gRacePlayers[order[i]].characterId * 6]
                );
            }
        } else if (gRacePlayers[order[i]].activeSparkleEffectCount != 0) {
            temp.assetTable = getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE);
            drawAssetTableSpriteWithExplicitPalette(
                xBase,
                (s16)(gRacePlayers[order[i]].progressMeterPosition + yBase),
                temp.assetTable,
                (&gRaceProgressMeterIconTiles[gRacePlayers[order[i]].progressMeterSquashFrame])[gRacePlayers[order[i]].characterId * 6],
                gRaceProgressMeterIconPalettes[gRacePlayers[order[i]].characterId]
            );
        } else {
            temp.assetTable = getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE);
            drawAssetTableSprite(
                xBase,
                (s16)(gRacePlayers[order[i]].progressMeterPosition + yBase),
                temp.assetTable,
                (&gRaceProgressMeterIconTiles[gRacePlayers[order[i]].progressMeterSquashFrame])[gRacePlayers[order[i]].characterId * 6]
            );
        }
        i--;
    } while (i >= 0);

    // @recomp Restore the original rectangle state after the progress meter.
    hudEndAnchoredDraw();
}
