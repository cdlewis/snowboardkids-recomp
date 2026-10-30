#include "jp_symbols.h"
#include "../../RecompiledPatchesJP/patches_bin.h"
#include "../../RecompiledPatchesJP/recomp_overlays.inl"
#include "librecomp/overlays.hpp"

namespace sk1 {
void register_jp_patches() {
    recomp::overlays::register_patches(jp_mm_patches_bin, sizeof(jp_mm_patches_bin), section_table, ARRLEN(section_table));
    recomp::overlays::register_base_exports(export_table);
    recomp::overlays::register_base_events(event_names);
    recomp::overlays::register_manual_patch_symbols(manual_patch_symbols);
}
}
