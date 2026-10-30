#include "game_regions.h"

#include <array>
#include <cstdlib>
#include <fstream>
#include <string_view>
#include <vector>
#include "ovl_patches.hpp"
#include "ultramodern/ultramodern.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <spawn.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
extern char** environ;
#endif

namespace sk1 {
void register_jp_overlays();
void register_jp_patches();

namespace {
std::optional<std::u8string> registered_game;
std::optional<std::u8string> restart_game;
std::filesystem::path executable;
std::vector<std::string> arguments;
constexpr std::string_view resume_argument = "--resume-game=";

bool relaunch_game(const std::u8string& game_id) {
    const std::string resume = std::string(resume_argument) + reinterpret_cast<const char*>(game_id.c_str());
#ifdef _WIN32
    std::wstring command = GetCommandLineW();
    command += L" " + std::wstring(resume.begin(), resume.end());
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup,
                        &process)) {
        return false;
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
#else
    auto child_arguments = arguments;
    child_arguments.push_back(resume);
    std::vector<char*> argv;
    for (auto& argument : child_arguments)
        argv.push_back(argument.data());
    argv.push_back(nullptr);
    pid_t child;
    if (posix_spawn(&child, executable.c_str(), nullptr, nullptr, argv.data(), environ) != 0) {
        return false;
    }
#endif
    // Quit through the app's existing save-flushing exit path after spawning successfully.
    ultramodern::quit();
    return true;
}
} // namespace

void initialize_game_restart(int argc, char** argv) {
    for (int index = 0; index < argc; ++index) {
        std::string_view argument{ argv[index] };
        if (argument.starts_with(resume_argument)) {
            auto id = argument.substr(resume_argument.size());
            restart_game = std::u8string(id.begin(), id.end());
        } else {
            arguments.emplace_back(argument);
        }
    }
#ifdef _WIN32
    std::wstring path(32768, L'\0');
    auto length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    path.resize(length);
    executable = path;
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> path(size);
    if (_NSGetExecutablePath(path.data(), &size) == 0)
        executable = path.data();
#else
    std::error_code error;
    executable = std::filesystem::read_symlink("/proc/self/exe", error);
    // Reopen an AppImage through its runtime so its assets retain a live mount.
    if (const char* appimage = std::getenv("APPIMAGE"))
        executable = appimage;
#endif
#ifndef _WIN32
    if (!arguments.empty())
        arguments[0] = executable.string();
#endif
}

std::optional<std::u8string> take_restart_game() {
    auto requested = std::move(restart_game);
    restart_game.reset();
    return requested;
}

recomp::RomValidationError import_rom(const std::filesystem::path& path, std::span<const recomp::GameEntry> games,
                                      std::u8string& game_id) {
    // The runtime reads header fields when reporting a region mismatch; reject short files first.
    std::ifstream file{ path, std::ios::binary };
    if (!file)
        return recomp::RomValidationError::FailedToOpen;
    std::array<char, 0x40> header;
    if (!file.read(header.data(), header.size()))
        return recomp::RomValidationError::NotARom;

    file.close();

    auto error = recomp::RomValidationError::IncorrectRom;
    for (const auto& game : games) {
        auto candidate = game.game_id;
        auto result = recomp::select_rom(path, candidate);
        if (result == recomp::RomValidationError::Good) {
            recomp::check_all_stored_roms();
            if (!recomp::is_rom_valid(candidate))
                return recomp::RomValidationError::OtherError;
            game_id = candidate;
            return result;
        }
        if (result == recomp::RomValidationError::FailedToOpen || result == recomp::RomValidationError::NotARom) {
            return result;
        }
        if (result != recomp::RomValidationError::IncorrectRom)
            error = result;
    }
    return error;
}

bool start_game(const std::u8string& game_id) {
    if (game_id != u8"snowboardkids.n64.us" && game_id != u8"snowboardkids.n64.jp")
        return false;
    if (registered_game && *registered_game != game_id)
        return relaunch_game(game_id);
    if (!registered_game) {
        if (game_id == u8"snowboardkids.n64.jp") {
            register_jp_overlays();
            register_jp_patches();
        } else {
            zelda64::register_overlays();
            zelda64::register_patches();
        }
        registered_game = game_id;
    }
    recomp::start_game(game_id, {});
    return true;
}
} // namespace sk1
