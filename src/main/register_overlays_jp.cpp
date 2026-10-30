#include "jp_symbols.h"
#include "../../RecompiledFuncsJP/recomp_overlays.inl"
#include "librecomp/overlays.hpp"

namespace sk1 {
void register_jp_overlays() {
    recomp::overlays::register_overlays(
        {section_table, ARRLEN(section_table), num_sections},
        {overlay_sections_by_index, ARRLEN(overlay_sections_by_index)});
}
}
