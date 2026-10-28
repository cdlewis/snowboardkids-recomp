# Windows crash reports

Windows builds bundle Crashpad's separate `crashpad_handler.exe`. It starts at the beginning of `main`, before
SDL, audio, the launcher, and the game runtime. If the helper is missing or cannot start, the game continues and logs
the failure to stderr (redirect stderr to a file to diagnose early startup failures). This cannot capture failures before
`main`, forced process termination, or every kind of system/driver failure. A frozen application does not automatically
generate a report.

Reports are saved under `%LOCALAPPDATA%\SnowboardKidsRecompiled\crash-reports`. Portable mode uses `crash-reports`
inside the application's existing portable data directory instead. The helper is always resolved beside the executable,
regardless of the working directory. Keep it with the game when moving the installation.

Nothing is uploaded: the handler has no upload URL, the database's upload setting is reset to disabled on every launch,
and the handler's periodic tasks are disabled. No account or crash-reporting service is required.

On the next launch after a crash, a Windows dialog offers to open the report folder. Choosing either Yes or No
acknowledges the existing reports, so they do not prompt again. Find the newest `.dmp` file inside the folder (including
its subfolders) and share it privately with the developer alongside reproduction steps. Dumps can contain fragments of
memory and personal information. Do not automatically post them to a public issue. No ROM, save, or log files are
explicitly attached, but a minidump can still contain fragments of application data.

Startup pruning keeps up to ten reports, no older than thirty days, with at most 64 MiB of report data in total.
These limits are applied at launch; a newly generated report can temporarily exceed them until the next launch.
Crashpad's database APIs perform deletion so report metadata is kept consistent.

Each dump records the product, game version, and full Git commit used at CMake configuration time (`unknown` for
source archives without Git metadata). Uncommitted changes are not identified by the commit hash. Retain the exact
Windows executable and its matching `.pdb` for every distributed build. CI already archives the PDB separately, and the
release workflow publishes it. Analyze a received dump with those symbols in WinDbg or Visual Studio; symbols from a
different build will not reliably resolve source locations. GitHub Actions artifacts expire, so use the release assets
for long-term retention.

## Validation

After configuring Windows as described in [BUILDING.md](../BUILDING.md), run:

```powershell
cmake --build cmake-build --target SnowboardKidsRecompiled WindowsCrashReportsTest
ctest --test-dir cmake-build -R WindowsCrashReports --output-on-failure
```

For a capture-only build without a ROM or submodules, configure the standalone test instead:

```powershell
cmake -S tests/windows-crash-reports -B build-crash-test -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
cmake --build build-crash-test
ctest --test-dir build-crash-test --output-on-failure
```

The smoke test runs a child process that raises an unhandled Windows exception, using a fresh Unicode data path and a
working directory different from the executable's. It checks the crash exit code, minidump signature, product/version
annotations, disabled uploads, and startup pruning to ten reports. It does not launch the game or touch real saves.
Its temporary crash database remains under the test account's temporary directory for inspection. Run this test
manually when changing crash reporting; it is not part of the CI workflow.

Manually verify the next-launch dialog on Windows using a disposable data directory: generate a report with the test,
then run the game against that database in portable mode. Check Yes opens Explorer, No allows startup to continue,
and an acknowledged report does not prompt again. Also test a copied package without its helper: the game should still
launch with crash reporting disabled. Test normal launcher/game startup with the packaged helper present, and confirm
the helper exits when the game closes. The smoke test alone does not establish game runtime or dialog behavior.

## Dependency and notices

`vcpkg.json` and the Windows CI checkout pin vcpkg to `866e9e258a76437074a5cf02d4c2d71b53bff2f8`, which pins
Crashpad to `efdc820b087c20eec9e32cb5e5b1a63dcf73a724` and mini_chromium to
`e5169551c51f3a52eee36b3b03f219cefe380237`. The packaged notices include Crashpad, mini_chromium, ICU, getopt, and zlib.
There is no new submodule or change to existing dependency gitlinks.

References: [Crashpad design](https://github.com/chromium/crashpad/blob/main/doc/overview_design.md),
[Crashpad client API](https://github.com/chromium/crashpad/blob/main/client/crashpad_client.h), and
[vcpkg Crashpad port](https://github.com/microsoft/vcpkg/tree/866e9e258a76437074a5cf02d4c2d71b53bff2f8/ports/crashpad).
