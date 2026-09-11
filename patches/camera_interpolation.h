#pragma once

#include "patches.h"

// Return the main projection cut decision so camera-facing model transforms snap with the camera.
s32 viewportCameraSkipsInterpolation(void);

// Invalidate both projection histories when a scripted jump replaces this viewport camera.
void invalidateViewportCameraInterpolation(u32 viewportIndex);
