#include "patches.h"
#include "race_split_screen.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/items/race_item_effects.h"

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

extern u8 gCurrentViewportIndex;

RECOMP_PATCH void renderRaceUiSparkle(RaceItemEffectActor *arg0) {
    if ((u8)arg0->payload.sprite.colorR == gCurrentViewportIndex) {
        // @recomp colorG selects half-size multiplayer sparkles, use the item HUD's right-edge anchor.
        if ((u8)arg0->payload.sprite.colorG != 0) {
            // @recomp Match the grid item boxes' right HUD anchor.
            anchorGridHudGroup(1);
            // @recomp Full-size vertical sparkles follow the vertical item-box anchor.
        } else if (raceUsesVerticalTwoPlayerSplit()) {
            // @recomp Match the vertical item boxes' right HUD anchor.
            anchorVerticalHudGroup(1);
        }

        if ((u8)arg0->payload.sprite.colorG == 0) {
            if ((u8)arg0->payload.sprite.colorB == 0) {
                drawAssetTableSpriteWithExplicitPalette(
                    arg0->payload.sprite.x,
                    arg0->payload.sprite.y,
                    getRelocatableHeapBlockBase(ASSET_HANDLE(0x1F)),
                    (arg0->payload.sprite.frame >> 1) + 0x5C,
                    0x1D
                );
            } else {
                drawAssetTableSpriteWithExplicitPalette(
                    arg0->payload.sprite.x,
                    arg0->payload.sprite.y,
                    getRelocatableHeapBlockBase(ASSET_HANDLE(0x1F)),
                    (arg0->payload.sprite.frame >> 1) + 0x5C,
                    0x1E
                );
            }
        } else if ((u8)arg0->payload.sprite.colorB == 0) {
            drawScaledAssetTableSpriteWithExplicitPalette(
                (s16)(arg0->payload.sprite.x - 8),
                (s16)(arg0->payload.sprite.y - 8),
                getRelocatableHeapBlockBase(ASSET_HANDLE(0x1F)),
                (arg0->payload.sprite.frame >> 1) + 0x5C,
                0x1D,
                1
            );
        } else {
            drawScaledAssetTableSpriteWithExplicitPalette(
                (s16)(arg0->payload.sprite.x - 8),
                (s16)(arg0->payload.sprite.y - 8),
                getRelocatableHeapBlockBase(ASSET_HANDLE(0x1F)),
                (arg0->payload.sprite.frame >> 1) + 0x5C,
                0x1E,
                1
            );
        }
        // @recomp Restore the viewport-centred rectangle alignment for subsequent overlay callbacks.
        if ((u8)arg0->payload.sprite.colorG != 0) {
            // @recomp Reset the grid viewport's rectangle origin and offset.
            anchorGridHudGroup(0);
            // @recomp Restore centred alignment after drawing a vertical sparkle.
        } else if (raceUsesVerticalTwoPlayerSplit()) {
            // @recomp Reset the vertical viewport's rectangle origin and offset.
            anchorVerticalHudGroup(0);
        }
    }
}

RECOMP_PATCH void spawnRaceUiSparkle(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4) {
    RaceItemEffectActor *temp_v0;

    // @recomp arg3 selects half-size grid sparkles, whose item boxes moved 56 pixels right.
    if (arg3 != 0) {
        // @recomp Follow the grid item boxes' horizontal offset.
        arg0 += 0x38;
        // @recomp Full-size vertical sparkles must follow the relocated top-right item boxes.
    } else if (raceUsesVerticalTwoPlayerSplit()) {
        // @recomp Follow the vertical item boxes from X -136 to X 8.
        arg0 += 0x90;
        // @recomp Follow the vertical item boxes from their original -48 Y to the eight-pixel top inset at -112.
        arg1 -= 0x40;
    }

    temp_v0 = createCallbackTaskPreservingArgs((CallbackTaskCallback)initRaceUiSparkle, 5, 3);
    if (temp_v0 != NULL) {
        temp_v0->payload.sprite.x = arg0 - 8;
        temp_v0->payload.sprite.y = arg1 - 8;
        temp_v0->payload.sprite.colorR = arg2;
        temp_v0->payload.sprite.colorG = arg3;
        temp_v0->payload.sprite.colorB = arg4;
    }
}
