# Building Guide

This guide will help you build the project on your local machine. You will need to provide:

- A copy of the North American N64 release of Snowboard Kids as `snowboardkids.z64` in the repository root.
- A copy of the Japanese N64 release of Snowboard Kids as `snowboardkids.jp.z64` in the repository root.

Neither file is distributed with this repository.

These steps cover: generating the symbol dumps, running the recompiler, and finally building the project.

## 1. Clone the snowboardkids-recomp Repository

This project makes use of submodules so you will need to clone the repository with the `--recurse-submodules` flag.

```bash
git clone --recurse-submodules https://github.com/cdlewis/snowboardkids-recomp.git
# if you forgot to clone with --recurse-submodules
# cd /path/to/cloned/repo && git submodule update --init --recursive
```

## 2. Install Dependencies

### Mac

For Mac you will need Xcode installed along with the Metal toolchain:

```bash
xcodebuild -downloadComponent MetalToolchain
```

Along with the following dependencies:

```bash
brew install cmake ninja sdl2 gtk+3 lld llvm
```

### Linux

For Linux the instructions for Ubuntu are provided, but you can find the equivalent packages for your preferred distro.

```bash
sudo apt-get install cmake ninja-build libsdl2-dev libgtk-3-dev lld llvm clang
```

### Windows

You will need to install [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/).
In the setup process you'll need to select the following options and tools for installation:

- Desktop development with C++
- C++ Clang Compiler for Windows
- C++ CMake tools for Windows

The other tool necessary will be `make`, which can be installed via [Chocolatey](https://chocolatey.org/):

```bash
choco install make
```

Windows builds also require Crashpad through the pinned [vcpkg](https://github.com/microsoft/vcpkg) manifest.
In PowerShell, create a local vcpkg checkout (the directory is ignored):

```powershell
git clone https://github.com/microsoft/vcpkg.git build-vcpkg
git -C build-vcpkg checkout 866e9e258a76437074a5cf02d4c2d71b53bff2f8
./build-vcpkg/bootstrap-vcpkg.bat -disableMetrics
$env:VCPKG_ROOT = "$pwd/build-vcpkg"
```

Launch Visual Studio from that shell so `CMakeSettings.json` can resolve `VCPKG_ROOT`. For a command-line build,
add these arguments to the CMake configure command:

```powershell
-DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
```

CMake installs the manifest dependencies automatically. Crashpad and zlib are linked statically with the dynamic
MSVC runtime, matching the application. The build copies `crashpad_handler.exe` and its license notices beside the game;
include both when distributing a Windows build. Linux and macOS builds do not use Crashpad.
See [Windows crash reports](docs/windows-crash-reports.md) for reporting and validation instructions.

## 3. Build N64Recomp

Build [N64: Recompiled](https://github.com/N64Recomp/N64Recomp) (targets `N64RecompCLI` and `RSPRecomp`) and copy the
resulting `N64Recomp` and `RSPRecomp` executables to the root of this repository. Building instructions are
[here](https://github.com/N64Recomp/N64Recomp?tab=readme-ov-file#building).

CI pins N64Recomp to commit `81213c1831fab2521a6a5459c67b63437d67e253`; use the same commit if you hit unexpected
recompiler behaviour.

## 4. Symbol Dumps

The recompiler reads `snowboardkids-recomp-syms/dump.toml` and `snowboardkids-recomp-syms/data_dump.toml` for the US
version, and the equivalent files in `snowboardkids-recomp-syms/jp/` for the Japanese version. They are
checked into the `snowboardkids-recomp-syms` submodule, so a recursive checkout already has them and there is nothing to
generate. Updating them for a new decompilation build is a commit in that submodule plus a gitlink bump here.

## 5. Generate the C Code

```bash
python3 scripts/generate-code.py
```

This produces `RecompiledFuncs/`, `RecompiledFuncsJP/`, and `rsp/aspMain.cpp`. The patch recompilers (`patches.toml`
and `patches.jp.toml`) are run automatically as part of the CMake build.

## 6. Build the Project

On Windows, you can open the repository folder with Visual Studio, and you'll be able to `[build / run / debug]` the
project from there.

If you prefer the command line or you're on a Unix platform you can build the project using CMake:

```bash
cmake -S . -B build-cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake --target SnowboardKidsRecompiled -j$(nproc) --config Release
```

`scripts/clean-build.sh` wraps a full from-scratch build including the patch payload.

The version string is taken from the `GAME_VERSION` environment variable, or derived from the newest reachable `vX.Y.Z`
git tag if it is unset. On a fresh clone with no tags, set it explicitly:

```bash
GAME_VERSION=0.1.0 cmake --build build-cmake --target SnowboardKidsRecompiled -j8
```

> [!IMPORTANT]
> On macOS, do not use Apple clang for the MIPS patch build; it does not support the required MIPS flags. CMake selects
> Homebrew LLVM automatically when it is present, or you can pass `-DPATCHES_C_COMPILER` and `-DPATCHES_LD` explicitly.

## 7. Success

You should now have a `SnowboardKidsRecompiled` executable in the build directory (a `.app` bundle on macOS). You will
need to run the executable out of the root folder of this project or copy the `assets` folder next to it.
