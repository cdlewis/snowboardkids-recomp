#include "patches.h"
#include "game/menu/renderer/menu_render_utils.h"

extern Gfx *gRegionAllocPtr;
extern Gfx gMenuRenderModeResetDl[];
extern s16 gMenuViewportWidth;
extern s16 gMenuViewportHeight;
extern s16 gMenuViewportCenterX;
extern s16 gMenuViewportCenterY;

RECOMP_PATCH void drawMenuFillRectangle(s16 x, s16 y, s16 width, s16 height, u8 red, u8 green, u8 blue) {
    volatile char pad[0x18];
    s32 minY;
    s32 maxY;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 x0;
    s32 minX;
    s32 halfHeight;
    s32 halfWidth;
    s32 maxX;

    x0 = x + gMenuViewportCenterX;
    y0 = y + gMenuViewportCenterY;
    x1 = x0 + width;
    y1 = y0 + height;

    halfWidth = gMenuViewportWidth / 2; maxX = gMenuViewportCenterX + halfWidth;
    if (x0 < maxX) {
        do {
            halfHeight = gMenuViewportHeight / 2;
        } while (0);
        maxY = gMenuViewportCenterY + halfHeight;
        minX = gMenuViewportCenterX - halfWidth;
        if (y0 < maxY) {
            minY = gMenuViewportCenterY - halfHeight;
            if ((x1 >= minX) && (y1 >= minY)) {
                if (x0 < minX) {
                    x0 = minX;
                }
                if (y0 < minY) {
                    y0 = minY;
                }
                if (x1 >= maxX) {
                    x1 = maxX;
                }
                if (y1 >= maxY) {
                    y1 = maxY;
                }

                gDPPipeSync(gRegionAllocPtr++);
                // @recomp Draw the item-training backgrounds as opaque primitives so RT64 does not issue partial clears
                gDPSetCycleType(gRegionAllocPtr++, G_CYC_1CYCLE);
                gDPSetRenderMode(gRegionAllocPtr++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
                gDPSetCombineMode(gRegionAllocPtr++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
                // @recomp Expand the original fill color's five-bit channels to preserve its RGB555 quantization.
                gDPSetPrimColor(gRegionAllocPtr++, 0, 0, (red & 0xF8) | (red >> 5),
                               (green & 0xF8) | (green >> 5), (blue & 0xF8) | (blue >> 5), 255);
                // @recomp One-cycle rectangles use exclusive lower-right bounds instead of fill-cycle inclusive bounds.
                gDPFillRectangle(gRegionAllocPtr++, x0, y0, x1, y1);
                gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
            }
        }
    }
}
