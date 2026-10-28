#include "sk1_crash_reports.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "client/crash_report_database.h"
#include "client/settings.h"

namespace {

int fail(const char* message) {
    std::fprintf(stderr, "%s\n", message);
    return 1;
}

} // namespace

int wmain(int argc, wchar_t** argv) {
    if (argc == 3 && std::wstring(argv[1]) == L"--crash") {
        if (!sk1::crash_reports::initialize(argv[2], "0.0.0-test")) {
            return 2;
        }
        RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, nullptr);
        return 3;
    }

    if (sk1::crash_reports::initialize({}, "0.0.0-test")) {
        return fail("An unavailable data directory must disable crash reporting.");
    }

    // Use a fresh Unicode path and a different working directory to exercise Windows path handling.
    const auto folder =
        std::filesystem::temp_directory_path() /
        (L"sk1-crash-test-\u96ea-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()));
    std::filesystem::create_directories(folder);
    std::wstring executable(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
    if (length == 0 || length >= executable.size()) {
        return fail("Could not resolve the test executable.");
    }
    executable.resize(length);
    std::wstring command = L"\"" + executable + L"\" --crash \"" + folder.native() + L"\"";
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr, folder.c_str(),
                        &startup, &process)) {
        return fail("Could not start the crashing child.");
    }
    CloseHandle(process.hThread);
    const DWORD wait = WaitForSingleObject(process.hProcess, 30000);
    if (wait != WAIT_OBJECT_0) {
        TerminateProcess(process.hProcess, 1);
        CloseHandle(process.hProcess);
        return fail("Crash handling did not finish within 30 seconds.");
    }
    DWORD exit_code = 0;
    GetExitCodeProcess(process.hProcess, &exit_code);
    CloseHandle(process.hProcess);
    if (exit_code != EXCEPTION_ACCESS_VIOLATION) {
        return fail("Child did not exit with the expected access violation.");
    }

    auto database = crashpad::CrashReportDatabase::Initialize(base::FilePath((folder / L"crash-reports").native()));
    if (!database) {
        return fail("Could not open the crash database.");
    }
    bool uploads = true;
    if (!database->GetSettings()->GetUploadsEnabled(&uploads) || uploads) {
        return fail("Crash uploads must remain disabled.");
    }
    std::vector<crashpad::CrashReportDatabase::Report> reports;
    if (database->GetPendingReports(&reports) != crashpad::CrashReportDatabase::kNoError || reports.size() != 1) {
        return fail("Expected exactly one pending local crash report.");
    }
    std::ifstream dump(std::filesystem::path(reports.front().file_path.value()), std::ios::binary);
    char signature[4]{};
    dump.read(signature, sizeof(signature));
    if (std::string(signature, 4) != "MDMP") {
        return fail("Crash report is not a minidump.");
    }
    const std::string contents{ std::istreambuf_iterator<char>(dump), std::istreambuf_iterator<char>() };
    if (contents.find("0.0.0-test") == std::string::npos ||
        contents.find("SnowboardKidsRecompiled") == std::string::npos) {
        return fail("Minidump is missing the application version or product annotation.");
    }
    dump.close();
    // Seed extra reports through Crashpad's API, then check startup retention across pending and completed reports.
    for (int i = 0; i < 12; ++i) {
        std::unique_ptr<crashpad::CrashReportDatabase::NewReport> report;
        crashpad::UUID uuid;
        if (database->PrepareNewCrashReport(&report) != crashpad::CrashReportDatabase::kNoError ||
            !report->Writer()->Write("MDMP", 4) ||
            database->FinishedWritingCrashReport(std::move(report), &uuid) != crashpad::CrashReportDatabase::kNoError) {
            return fail("Could not seed reports for the retention test.");
        }
        if (i % 2 == 0 && database->SkipReportUpload(uuid, crashpad::Metrics::CrashSkippedReason::kUploadsDisabled) !=
                              crashpad::CrashReportDatabase::kNoError) {
            return fail("Could not mark a seeded report as completed without uploading.");
        }
    }
    database.reset();
    // Reopening through the application must prune extra reports and keep uploads disabled.
    if (!sk1::crash_reports::initialize(folder, "0.0.0-test")) {
        return fail("Crash reporting failed on the next launch.");
    }
    database = crashpad::CrashReportDatabase::Initialize(base::FilePath((folder / L"crash-reports").native()));
    reports.clear();
    std::vector<crashpad::CrashReportDatabase::Report> completed;
    if (!database || database->GetPendingReports(&reports) != crashpad::CrashReportDatabase::kNoError ||
        database->GetCompletedReports(&completed) != crashpad::CrashReportDatabase::kNoError ||
        reports.size() + completed.size() != 10) {
        return fail("Startup retention must keep only ten reports.");
    }
    if (!database->GetSettings()->GetUploadsEnabled(&uploads) || uploads) {
        return fail("Uploads were enabled on restart.");
    }
    std::printf("PASS: captured a Windows crash locally, with uploads disabled and Unicode paths.\n");
    return 0;
}
