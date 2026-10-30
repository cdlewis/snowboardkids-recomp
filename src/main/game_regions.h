#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include "librecomp/game.hpp"

namespace sk1 {
// Import a supported ROM and return its detected game ID on success.
recomp::RomValidationError import_rom(const std::filesystem::path& path, std::span<const recomp::GameEntry> games,
                                      std::u8string& game_id);
bool start_game(const std::u8string& game_id);
void initialize_game_restart(int argc, char** argv);
std::optional<std::u8string> take_restart_game();
} // namespace sk1
