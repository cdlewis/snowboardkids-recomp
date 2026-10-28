#pragma once

#include <filesystem>

namespace sk1::crash_reports {

// Windows only. Failures leave the game usable and are logged to stderr.
bool initialize(const std::filesystem::path& app_folder, const char* version) noexcept;
void notify_previous_crash() noexcept;

} // namespace sk1::crash_reports
