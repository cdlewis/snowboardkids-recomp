#pragma once
#include <span>

namespace recomp {
struct GameEntry;
}

namespace sk1::launcher {

void register_callbacks(std::span<const recomp::GameEntry> games);

} // namespace sk1::launcher
