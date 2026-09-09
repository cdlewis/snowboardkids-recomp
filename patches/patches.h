#pragma once

#define RECOMP_EXPORT __attribute__((section(".recomp_export")))
#define RECOMP_PATCH __attribute__((section(".recomp_patch")))
#define RECOMP_FORCE_PATCH __attribute__((section(".recomp_force_patch")))
#define RECOMP_DECLARE_EVENT(func)                                                          \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
        __attribute__((noinline, weak, used, section(".recomp_event"))) void func {         \
    }                                                                                       \
    _Pragma("GCC diagnostic pop")

// Redirect libultra calls to the reimplementations provided by librecomp and
// ultramodern. Patch code is compiled for MIPS and linked against the dummy
// addresses in syms.ld, which N64Recomp turns into native calls.
// TODO fix renaming symbols in patch recompilation
#define osCreateMesgQueue osCreateMesgQueue_recomp
#define osRecvMesg osRecvMesg_recomp
#define osSendMesg osSendMesg_recomp
#define osViGetCurrentFramebuffer osViGetCurrentFramebuffer_recomp
#define osViSwapBuffer osViSwapBuffer_recomp
#define osWritebackDCache osWritebackDCache_recomp
#define osWritebackDCacheAll osWritebackDCacheAll_recomp
#define osInvalICache osInvalICache_recomp
#define osGetTime osGetTime_recomp

#define osContStartReadData osContStartReadData_recomp
#define osContGetReadData osContGetReadData_recomp

#define __sinf __sinf_recomp
#define __cosf __cosf_recomp
#define sqrtf sqrtf_recomp
#define osPiStartDma osPiStartDma_recomp
#define osAiGetLength osAiGetLength_recomp
#define osAiSetNextBuffer osAiSetNextBuffer_recomp
#define osVirtualToPhysical osVirtualToPhysical_recomp

#include "PR/ultratypes.h"
// mbi.h defines _SHIFTL/_SHIFTR and then includes gbi.h and abi.h, which need them.
// Including gbi.h directly leaves the gDP/gSP macros referring to undefined symbols.
#include "PR/mbi.h"
#include "rt64_extended_gbi.h"
#include "PR/ucode.h"

#include "game/audio/audio_engine.h"
#include "game/engine/system_runtime.h"
#include "game/engine/frame_render_task.h"

// Native helpers backed by src/game/recomp_api.cpp; see patches/syms.ld.
int recomp_printf(const char* fmt, ...);
int _Sprintf(char* buffer, const char* fmt, ...);
float recomp_powf(float, float);
float recomp_get_target_aspect_ratio(float original);
f32 __sinf(f32);
f32 __cosf(f32);
float sqrtf(float f);

#define INCBIN(identifier, filename)         \
    asm(".pushsection .rodata\n"             \
        "\t.local " #identifier "\n"         \
        "\t.type " #identifier ", @object\n" \
        "\t.balign 8\n" #identifier ":\n"    \
        "\t.incbin \"" filename "\"\n\n"     \
                                             \
        "\t.balign 8\n"                      \
        "\t.popsection\n");                  \
    extern u8 identifier[]
