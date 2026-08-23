#include "zelda_support.h"
#include <SDL.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include "nfd.h"
#include "RmlUi/Core.h"
#include "ultramodern/ultramodern.hpp"

// librecomp's shutdown flag, set by ultramodern::quit(). It is a plain global with external
// linkage in librecomp/src/recomp.cpp and is not declared in any header; librecomp's own pi.cpp
// and ultramodern's events.cpp reach for it the same way.
extern std::atomic_bool exited;

namespace zelda64 {
    [[noreturn]] void quit_process_now() {
        // Quitting cannot be allowed to unwind through recomp::start. Once its loop sees
        // `exited` it joins its own bookkeeping threads and then unmaps rdram -- but nothing
        // joins the OSThreads the game created, and the thread cleaner only ever joins the ones
        // that terminate on their own. Those threads are still executing recompiled code, so
        // they read memory that has just been handed back to the OS, and afterwards run into
        // the namespace-scope globals that exit() destroys. Both are races whose outcome
        // depends on what the game's threads happen to be doing at the moment of the quit.
        //
        // There is nothing left to tear down that the OS will not reclaim: RT64's shutdown is
        // pure GPU-resource teardown with no persistent writes, and the save file is the only
        // durable state. The saving thread already sees `exited`, so joining it finishes the
        // write it has coalesced and returns immediately.
        ultramodern::join_saving_thread();
        std::fflush(nullptr);
        std::_Exit(EXIT_SUCCESS);
    }

    // MARK: - Internal Helpers
    void perform_file_dialog_operation(const std::function<void(bool, const std::filesystem::path&)>& callback) {
        nfdnchar_t* native_path = nullptr;
        nfdresult_t result = NFD_OpenDialogN(&native_path, nullptr, 0, nullptr);

        bool success = (result == NFD_OKAY);
        std::filesystem::path path;

        if (success) {
            path = std::filesystem::path{native_path};
            NFD_FreePathN(native_path);
        }

        callback(success, path);
    }

    void perform_file_dialog_operation_multiple(const std::function<void(bool, const std::list<std::filesystem::path>&)>& callback) {
        const nfdpathset_t* native_paths = nullptr;
        nfdresult_t result = NFD_OpenDialogMultipleN(&native_paths, nullptr, 0, nullptr);

        bool success = (result == NFD_OKAY);
        std::list<std::filesystem::path> paths;
        nfdpathsetsize_t count = 0;

        if (success) {
            NFD_PathSet_GetCount(native_paths, &count);
            for (nfdpathsetsize_t i = 0; i < count; i++) {
                nfdnchar_t* cur_path = nullptr;
                nfdresult_t cur_result = NFD_PathSet_GetPathN(native_paths, i, &cur_path);
                if (cur_result == NFD_OKAY) {
                    paths.emplace_back(std::filesystem::path{cur_path});
                }
            }
            NFD_PathSet_Free(native_paths);
        }

        callback(success, paths);
    }

    // MARK: - Public API

    std::filesystem::path get_program_path() {
#if defined(__APPLE__)
        return get_bundle_resource_directory();
#elif defined(__linux__) && defined(RECOMP_FLATPAK)
        return "/app/bin";
#else
        return "";
#endif
    }

    std::filesystem::path get_asset_path(const char* asset) {
        return get_program_path() / "assets" / asset;
    }

    void open_file_dialog(std::function<void(bool success, const std::filesystem::path& path)> callback) {
#ifdef __APPLE__
        dispatch_on_ui_thread([callback]() {
            perform_file_dialog_operation(callback);
        });
#else
        perform_file_dialog_operation(callback);
#endif
    }

    void open_file_dialog_multiple(std::function<void(bool success, const std::list<std::filesystem::path>& paths)> callback) {
#ifdef __APPLE__
        dispatch_on_ui_thread([callback]() {
            perform_file_dialog_operation_multiple(callback);
        });
#else
        perform_file_dialog_operation_multiple(callback);
#endif
    }

    void show_error_message_box(const char *title, const char *message) {
#ifdef __APPLE__
    std::string title_copy(title);
    std::string message_copy(message);

    dispatch_on_ui_thread([title_copy, message_copy] {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title_copy.c_str(), message_copy.c_str(), nullptr);
    });
#else
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, nullptr);
#endif
    }
}
