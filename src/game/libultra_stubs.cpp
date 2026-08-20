// Stubs for libultra functions that N64Recomp refuses to recompile (they are in
// its built-in ignored_funcs list) but that librecomp does not reimplement.
//
// Snowboard Kids' symbol dump contains all of these, so the recompiled game code
// calls them by their "_recomp" names and the executable will not link without
// them.
//
// All three Controller Pak internals sit underneath the public osPfs* API, which
// librecomp stubs out to PFS_ERR_NOPACK (see lib/N64ModernRuntime/librecomp/src/
// pak.cpp). They are therefore unreachable in practice; returning an error keeps
// them consistent with that layer rather than silently reporting success.

#include "recomp.h"

extern "C" void __osContRamRead_recomp(uint8_t* rdram, recomp_context* ctx) {
    ctx->r2 = 1; // PFS_ERR_NOPACK
}

extern "C" void __osContRamWrite_recomp(uint8_t* rdram, recomp_context* ctx) {
    ctx->r2 = 1; // PFS_ERR_NOPACK
}

extern "C" void __osPfsSelectBank_recomp(uint8_t* rdram, recomp_context* ctx) {
    ctx->r2 = 1; // PFS_ERR_NOPACK
}

// Debug-only printf from libultra's rmon. The retail game never reaches it.
extern "C" void rmonPrintf_recomp(uint8_t* rdram, recomp_context* ctx) {
    ctx->r2 = 0;
}
