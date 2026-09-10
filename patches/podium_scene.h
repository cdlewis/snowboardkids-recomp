#pragma once

#include "game/race/camera/race_camera.h"

static inline s32 isPodiumViewport(s32 index) {
    // Only initPassAwardCelebration assigns the static-follow camera to all three
    // podium viewports. Identify the scene by its cameras, not its inset bounds.
    return index < 3 && gRaceCameras[0].initialized.value == 0 &&
           gRaceCameras[0].mode == 0x1D && gRaceCameras[1].mode == 0x1D && gRaceCameras[2].mode == 0x1D;
}
