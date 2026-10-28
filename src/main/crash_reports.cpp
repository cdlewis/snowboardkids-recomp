#include "sk1_crash_reports.h"

#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

#include "client/crashpad_client.h"
#include "client/crash_report_database.h"
#include "client/prune_crash_reports.h"
#include "client/settings.h"
#include "sk1_game_version.h"

namespace {

std::unique_ptr<crashpad::CrashReportDatabase> database;
crashpad::CrashpadClient client;
std::filesystem::path database_path;

class ReportRetention final : public crashpad::PruneCondition {
  public:
    bool ShouldPruneReport(const crashpad::CrashReportDatabase::Report& report) override {
        // Crashpad visits newest reports first. Keep at most ten, 64 MiB in total, for thirty days.
        const bool expired = age.ShouldPruneReport(report);
        if (expired || count >= 10 || report.total_size > max_bytes - bytes) {
            return true;
        }
        ++count;
        bytes += report.total_size;
        return false;
    }

  private:
    crashpad::AgePruneCondition age{ 30 };
    static constexpr uint64_t max_bytes = 64 * 1024 * 1024;
    uint64_t bytes = 0;
    size_t count = 0;
};

std::filesystem::path executable_folder() {
    // GetModuleFileNameW resolves the helper beside the executable, independently of the working directory.
    std::wstring path(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (length == 0 || length >= path.size()) {
        return {};
    }
    path.resize(length);
    return std::filesystem::path(path).parent_path();
}

} // namespace

bool sk1::crash_reports::initialize(const std::filesystem::path& app_folder, const char* version) noexcept {
    try {
        if (database) {
            return true;
        }
        const auto executable_dir = executable_folder();
        if (app_folder.empty() || executable_dir.empty()) {
            std::fprintf(stderr, "Crash reports disabled: application path unavailable.\n");
            return false;
        }
        const auto handler_path = executable_dir / L"crashpad_handler.exe";
        if (!std::filesystem::is_regular_file(handler_path)) {
            std::fprintf(stderr, "Crash reports disabled: crashpad_handler.exe is missing.\n");
            return false;
        }
        database_path = std::filesystem::absolute(app_folder / L"crash-reports");
        std::filesystem::create_directories(database_path);
        auto new_database = crashpad::CrashReportDatabase::Initialize(base::FilePath(database_path.native()));
        if (!new_database || !new_database->GetSettings()->SetUploadsEnabled(false)) {
            std::fprintf(stderr, "Crash reports disabled: cannot initialize the local database.\n");
            return false;
        }
        ReportRetention retention;
        crashpad::PruneCrashReportDatabase(new_database.get(), &retention);

        // No upload URL, uploads explicitly disabled, and no background upload or pruning tasks.
        // Prune on startup using our limits instead of the handler's default retention policy.
        const std::map<std::string, std::string> annotations{ { "prod", "SnowboardKidsRecompiled" },
                                                              { "ver", version },
                                                              { "build", sk1_build_id } };
        if (!client.StartHandler(base::FilePath(handler_path.native()), base::FilePath(database_path.native()),
                                 base::FilePath(), "", annotations, { "--no-periodic-tasks" }, true, false)) {
            std::fprintf(stderr, "Crash reports disabled: handler startup failed.\n");
            return false;
        }
        database = std::move(new_database);
        return true;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Crash reports disabled: %s\n", error.what());
        return false;
    }
}

void sk1::crash_reports::notify_previous_crash() noexcept {
    try {
        if (!database) {
            return;
        }
        std::vector<crashpad::CrashReportDatabase::Report> reports;
        std::vector<crashpad::CrashReportDatabase::Report> completed;
        if (database->GetPendingReports(&reports) != crashpad::CrashReportDatabase::kNoError ||
            database->GetCompletedReports(&completed) != crashpad::CrashReportDatabase::kNoError) {
            return;
        }
        reports.insert(reports.end(), completed.begin(), completed.end());
        std::ifstream input(database_path / L"notified.txt");
        const std::set<std::string> notified{ std::istream_iterator<std::string>(input),
                                              std::istream_iterator<std::string>() };
        const bool unseen = std::any_of(reports.begin(), reports.end(),
                                        [&](const auto& report) { return !notified.contains(report.uuid.ToString()); });
        if (!unseen) {
            return;
        }
        const int choice =
            MessageBoxW(nullptr,
                        L"A previous run of Snowboard Kids: Recompiled crashed. A crash report was saved locally.\n\n"
                        L"Nothing has been uploaded. Reports can contain fragments of memory and personal information. "
                        L"Share a .dmp file privately with the developer if you want help investigating.\n\n"
                        L"Open the crash reports folder?",
                        L"Snowboard Kids: Recompiled crash report", MB_YESNO | MB_ICONINFORMATION | MB_DEFBUTTON2);
        if (choice == 0) {
            return;
        }
        if (choice == IDYES) {
            const auto result = reinterpret_cast<INT_PTR>(
                ShellExecuteW(nullptr, L"open", database_path.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
            if (result <= 32) {
                std::fprintf(stderr, "Could not open the crash reports folder.\n");
                return;
            }
        }
        // Acknowledge both Yes and No so the same crash does not interrupt every subsequent launch.
        std::ofstream output(database_path / L"notified.txt", std::ios::trunc);
        for (const auto& report : reports) {
            output << report.uuid.ToString() << '\n';
        }
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Could not display previous crash reports: %s\n", error.what());
    }
}
