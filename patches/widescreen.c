#include "patches.h"
#include "game/engine/viewport_manager.h"
#include "PR/gu.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define VIEWPORT_EDGE_SNAP 16

extern const f32 gDefaultViewportOverlayFarClip[1];
extern const f32 gCustomViewportOverlayFarClip[1];
extern const f32 gRaceViewportOverlayFarClip[1];
extern const f32 gMenuViewportFarClip[1];
extern const f32 gMenuViewportOverlayFarClip[4];

static s16 snapLowEdge(s16 value, s16 boundary) {
    if ((value > boundary) && ((value - boundary) <= VIEWPORT_EDGE_SNAP)) {
        return boundary;
    }
    return value;
}

static s16 snapHighEdge(s16 value, s16 boundary) {
    if ((value < boundary) && ((boundary - value) <= VIEWPORT_EDGE_SNAP)) {
        return boundary;
    }
    return value;
}

static void snapViewportBoundsToScreenEdges(ViewportState *viewport) {
    viewport->left = snapLowEdge(viewport->left, 0);
    viewport->left = snapLowEdge(viewport->left, SCREEN_WIDTH / 2);

    viewport->top = snapLowEdge(viewport->top, 0);
    viewport->top = snapLowEdge(viewport->top, SCREEN_HEIGHT / 2);

    viewport->right = snapHighEdge(viewport->right, SCREEN_WIDTH / 2);
    viewport->right = snapHighEdge(viewport->right, SCREEN_WIDTH);

    viewport->bottom = snapHighEdge(viewport->bottom, SCREEN_HEIGHT / 2);
    viewport->bottom = snapHighEdge(viewport->bottom, SCREEN_HEIGHT);

    viewport->viewport.vp.vtrans[0] = ((viewport->left + viewport->right) / 2) * 4;
    viewport->viewport.vp.vtrans[1] = ((viewport->top + viewport->bottom) / 2) * 4;
}

RECOMP_PATCH void configureViewport(
    s32 viewportIndex,
    s32 centerX,
    s32 centerY,
    u16 width,
    u16 height,
    u16 scaleX,
    u16 scaleY,
    f32 aspect
) {
    gViewportStates[viewportIndex].active = 1;
    gViewportStates[viewportIndex].viewport.vp.vtrans[0] = centerX * 4;
    gViewportStates[viewportIndex].viewport.vp.vtrans[1] = centerY * 4;
    gViewportStates[viewportIndex].viewport.vp.vscale[0] = scaleX * 2;
    gViewportStates[viewportIndex].viewport.vp.vscale[1] = scaleY * 2;

    gViewportStates[viewportIndex].left = centerX - (width / 2);
    gViewportStates[viewportIndex].top = centerY - (height / 2);
    gViewportStates[viewportIndex].right = (width / 2) + centerX;
    gViewportStates[viewportIndex].bottom = (height / 2) + centerY;
    gViewportStates[viewportIndex].left = gViewportStates[viewportIndex].left;
    gViewportStates[viewportIndex].top = gViewportStates[viewportIndex].top;
    gViewportStates[viewportIndex].right = gViewportStates[viewportIndex].right;
    gViewportStates[viewportIndex].bottom = gViewportStates[viewportIndex].bottom;
    gViewportStates[viewportIndex].screenBoundsValid = 1;

    if (gViewportStates[viewportIndex].right < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].bottom < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left >= 0x140) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].top >= 0xF0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left < 0) {
        gViewportStates[viewportIndex].left = 0;
    }
    if (gViewportStates[viewportIndex].top < 0) {
        gViewportStates[viewportIndex].top = 0;
    }
    // @recomp Preserve the framebuffer edge instead of clamping one pixel inside it.
    if (gViewportStates[viewportIndex].right > 0x140) {
        gViewportStates[viewportIndex].right = 0x140;
    }
    if (gViewportStates[viewportIndex].bottom > 0xF0) {
        gViewportStates[viewportIndex].bottom = 0xF0;
    }

    // @recomp Expand near-edge bounds and recenter the rendered viewport to match them.
    snapViewportBoundsToScreenEdges(&gViewportStates[viewportIndex]);
    
    guPerspective(
        &gViewportStates[viewportIndex].projectionMatrix,
        &gViewportStates[viewportIndex].perspectiveNorm,
        70.0f,
        aspect,
        10.0f,
        2800.0f,
        0.5f
    );
    guPerspective(
        &gViewportStates[viewportIndex].overlayProjectionMatrix,
        &gViewportStates[viewportIndex].overlayPerspectiveNorm,
        70.0f,
        aspect,
        10.0f,
        gDefaultViewportOverlayFarClip[0],
        0.5f
    );
}

RECOMP_PATCH void configureViewportWithFovAndFarClip(
    s32 viewportIndex,
    s32 centerX,
    s32 centerY,
    u16 width,
    u16 height,
    u16 scaleX,
    u16 scaleY,
    f32 aspect,
    s16 fovY,
    s32 farClip
) {
    gViewportStates[viewportIndex].active = 1;
    gViewportStates[viewportIndex].viewport.vp.vtrans[0] = centerX * 4;
    gViewportStates[viewportIndex].viewport.vp.vtrans[1] = centerY * 4;
    gViewportStates[viewportIndex].viewport.vp.vscale[0] = scaleX * 2;
    gViewportStates[viewportIndex].viewport.vp.vscale[1] = scaleY * 2;

    gViewportStates[viewportIndex].left = centerX - (width / 2);
    gViewportStates[viewportIndex].top = centerY - (height / 2);
    gViewportStates[viewportIndex].right = (width / 2) + centerX;
    gViewportStates[viewportIndex].bottom = (height / 2) + centerY;
    gViewportStates[viewportIndex].left = gViewportStates[viewportIndex].left;
    gViewportStates[viewportIndex].top = gViewportStates[viewportIndex].top;
    gViewportStates[viewportIndex].right = gViewportStates[viewportIndex].right;
    gViewportStates[viewportIndex].bottom = gViewportStates[viewportIndex].bottom;
    gViewportStates[viewportIndex].screenBoundsValid = 1;

    if (gViewportStates[viewportIndex].right < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].bottom < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left >= 0x140) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].top >= 0xF0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left < 0) {
        gViewportStates[viewportIndex].left = 0;
    }
    if (gViewportStates[viewportIndex].top < 0) {
        gViewportStates[viewportIndex].top = 0;
    }
    // @recomp Preserve the framebuffer edge instead of clamping one pixel inside it.
    if (gViewportStates[viewportIndex].right > 0x140) {
        gViewportStates[viewportIndex].right = 0x140;
    }
    if (gViewportStates[viewportIndex].bottom > 0xF0) {
        gViewportStates[viewportIndex].bottom = 0xF0;
    }

    // @recomp Expand near-edge bounds and recenter the rendered viewport to match them.
    snapViewportBoundsToScreenEdges(&gViewportStates[viewportIndex]);
   
    guPerspective(
        &gViewportStates[viewportIndex].projectionMatrix,
        &gViewportStates[viewportIndex].perspectiveNorm,
        (f32)fovY,
        aspect,
        10.0f,
        (f32)farClip,
        0.5f
    );
    guPerspective(
        &gViewportStates[viewportIndex].overlayProjectionMatrix,
        &gViewportStates[viewportIndex].overlayPerspectiveNorm,
        (f32)fovY,
        aspect,
        10.0f,
        gCustomViewportOverlayFarClip[0],
        0.5f
    );
}

RECOMP_PATCH void configureRaceViewport(
    s32 viewportIndex,
    s32 centerX,
    s32 centerY,
    u16 width,
    u16 height,
    u16 scaleX,
    u16 scaleY,
    f32 aspect
) {
    gViewportStates[viewportIndex].active = 1;
    gViewportStates[viewportIndex].viewport.vp.vtrans[0] = centerX * 4;
    gViewportStates[viewportIndex].viewport.vp.vtrans[1] = centerY * 4;
    gViewportStates[viewportIndex].viewport.vp.vscale[0] = scaleX * 2;
    gViewportStates[viewportIndex].viewport.vp.vscale[1] = scaleY * 2;

    gViewportStates[viewportIndex].left = centerX - (width / 2);
    gViewportStates[viewportIndex].top = centerY - (height / 2);
    gViewportStates[viewportIndex].right = (width / 2) + centerX;
    gViewportStates[viewportIndex].bottom = (height / 2) + centerY;
    gViewportStates[viewportIndex].left = gViewportStates[viewportIndex].left;
    gViewportStates[viewportIndex].top = gViewportStates[viewportIndex].top;
    gViewportStates[viewportIndex].right = gViewportStates[viewportIndex].right;
    gViewportStates[viewportIndex].bottom = gViewportStates[viewportIndex].bottom;
    gViewportStates[viewportIndex].screenBoundsValid = 1;

    if (gViewportStates[viewportIndex].right < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].bottom < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left >= 0x140) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].top >= 0xF0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left < 0) {
        gViewportStates[viewportIndex].left = 0;
    }
    if (gViewportStates[viewportIndex].top < 0) {
        gViewportStates[viewportIndex].top = 0;
    }
    // @recomp Preserve the framebuffer edge instead of clamping one pixel inside it.
    if (gViewportStates[viewportIndex].right > 0x140) {
        gViewportStates[viewportIndex].right = 0x140;
    }
    if (gViewportStates[viewportIndex].bottom > 0xF0) {
        gViewportStates[viewportIndex].bottom = 0xF0;
    }

    // @recomp Expand near-edge bounds and recenter the rendered viewport to match them.
    snapViewportBoundsToScreenEdges(&gViewportStates[viewportIndex]);
    
    guPerspective(
        &gViewportStates[viewportIndex].projectionMatrix,
        &gViewportStates[viewportIndex].perspectiveNorm,
        70.0f,
        aspect,
        10.0f,
        1000.0f,
        0.5f
    );
    guPerspective(
        &gViewportStates[viewportIndex].overlayProjectionMatrix,
        &gViewportStates[viewportIndex].overlayPerspectiveNorm,
        70.0f,
        aspect,
        10.0f,
        gRaceViewportOverlayFarClip[0],
        0.5f
    );
}

RECOMP_PATCH void configureMenuViewport(
    s32 viewportIndex,
    s32 centerX,
    s32 centerY,
    u16 width,
    u16 height,
    u16 scaleX,
    u16 scaleY,
    f32 aspect
) {
    gViewportStates[viewportIndex].active = 1;
    gViewportStates[viewportIndex].viewport.vp.vtrans[0] = centerX * 4;
    gViewportStates[viewportIndex].viewport.vp.vtrans[1] = centerY * 4;
    gViewportStates[viewportIndex].viewport.vp.vscale[0] = scaleX * 2;
    gViewportStates[viewportIndex].viewport.vp.vscale[1] = scaleY * 2;

    gViewportStates[viewportIndex].left = centerX - (width / 2);
    gViewportStates[viewportIndex].top = centerY - (height / 2);
    gViewportStates[viewportIndex].right = (width / 2) + centerX;
    gViewportStates[viewportIndex].bottom = (height / 2) + centerY;
    gViewportStates[viewportIndex].left = gViewportStates[viewportIndex].left;
    gViewportStates[viewportIndex].top = gViewportStates[viewportIndex].top;
    gViewportStates[viewportIndex].right = gViewportStates[viewportIndex].right;
    gViewportStates[viewportIndex].bottom = gViewportStates[viewportIndex].bottom;
    gViewportStates[viewportIndex].screenBoundsValid = 1;

    if (gViewportStates[viewportIndex].right < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].bottom < 0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left >= 0x140) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].top >= 0xF0) {
        gViewportStates[viewportIndex].screenBoundsValid = 0;
    }
    if (gViewportStates[viewportIndex].left < 0) {
        gViewportStates[viewportIndex].left = 0;
    }
    if (gViewportStates[viewportIndex].top < 0) {
        gViewportStates[viewportIndex].top = 0;
    }
    // @recomp Preserve the framebuffer edge instead of clamping one pixel inside it.
    if (gViewportStates[viewportIndex].right > 0x140) {
        gViewportStates[viewportIndex].right = 0x140;
    }
    if (gViewportStates[viewportIndex].bottom > 0xF0) {
        gViewportStates[viewportIndex].bottom = 0xF0;
    }

    // @recomp Expand near-edge bounds and recenter the rendered viewport to match them.
    snapViewportBoundsToScreenEdges(&gViewportStates[viewportIndex]);
    
    guPerspective(
        &gViewportStates[viewportIndex].projectionMatrix,
        &gViewportStates[viewportIndex].perspectiveNorm,
        70.0f,
        aspect,
        10.0f,
        gMenuViewportFarClip[0],
        0.5f
    );
    guPerspective(
        &gViewportStates[viewportIndex].overlayProjectionMatrix,
        &gViewportStates[viewportIndex].overlayPerspectiveNorm,
        70.0f,
        aspect,
        10.0f,
        gMenuViewportOverlayFarClip[0],
        0.5f
    );
}
