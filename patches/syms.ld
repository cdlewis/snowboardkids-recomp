__start = 0x80000000;

/* Dummy addresses that get recompiled into native function calls.
 *
 * Every symbol listed here must resolve to a real native function at link time
 * if a patch actually references it: N64Recomp emits referenced entries into
 * RecompiledPatches/recomp_overlays.inl as manual_patch_symbols. Unreferenced
 * entries are dropped, so this list can stay ahead of the patches that use it,
 * but it must never name something that has no native implementation in
 * src/game/recomp_api.cpp or in librecomp/ultramodern.
 */

/* src/game/recomp_api.cpp */
recomp_puts = 0x8F000000;
recomp_exit = 0x8F000004;
recomp_get_gyro_deltas = 0x8F000018;
recomp_get_target_aspect_ratio = 0x8F00001C;
recomp_get_target_hud_aspect_ratio = 0x8F000020;
recomp_powf = 0x8F000024;
recomp_get_target_framerate = 0x8F000028;
recomp_get_bgm_volume = 0x8F000030;
recomp_time_us = 0x8F000050;
recomp_get_mouse_deltas = 0x8F000080;
recomp_load_overlays = 0x8F000090;
recomp_get_camera_inputs = 0x8F00009C;
recomp_set_right_analog_suppressed = 0x8F0000A0;
recomp_get_inverted_axes = 0x8F0000A4;
recomp_high_precision_fb_enabled = 0x8F0000A8;
recomp_get_resolution_scale = 0x8F0000AC;
recomp_get_analog_inverted_axes = 0x8F0000B0;
recomp_get_window_resolution = 0x8F0000B4;
recomp_get_sfx_volume = 0x8F0000D4;
recomp_get_voice_volume = 0x8F0000D8;
recomp_get_invert_y_axis_mode = 0x8F0000E8;
recomp_get_vertical_2p_split_screen_enabled = 0x8F000110;
recomp_set_game_player_count = 0x8F00010C;
recomp_save_read = 0x8F000114;
recomp_save_write = 0x8F000118;
sqrtf_recomp = 0x8F0000B8;

/* librecomp / ultramodern reimplementations. Only libultra functions that are
 * actually present in Snowboard Kids are listed; see syms/dump.toml.
 */
osRecvMesg_recomp = 0x8F000010;
osSendMesg_recomp = 0x8F000014;
__sinf_recomp = 0x8F000038;
__cosf_recomp = 0x8F00003C;
osCreateMesgQueue_recomp = 0x8F000048;
osViGetCurrentFramebuffer_recomp = 0x8F000058;
osWritebackDCache_recomp = 0x8F000068;
osContStartReadData_recomp = 0x8F000070;
osContGetReadData_recomp = 0x8F000074;
osGetTime_recomp = 0x8F000088;
osInvalICache_recomp = 0x8F000094;
osViSwapBuffer_recomp = 0x8F0000BC;
osPiStartDma_recomp = 0x8F0000F0;
osAiGetLength_recomp = 0x8F0000FC;
osAiSetNextBuffer_recomp = 0x8F000100;
osVirtualToPhysical_recomp = 0x8F000104;
osWritebackDCacheAll_recomp = 0x8F0000C0;
