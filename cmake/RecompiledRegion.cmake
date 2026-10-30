# Build a region's generated game code and the shared patches against its symbols.
function(add_recompiled_region region)
    set(suffix "")
    set(symbols_dir "${CMAKE_SOURCE_DIR}/snowboardkids-recomp-syms")
    set(patch_dir "${CMAKE_SOURCE_DIR}/patches")
    set(patch_config "${CMAKE_SOURCE_DIR}/patches.toml")
    if(NOT region STREQUAL "us")
        string(TOUPPER "${region}" suffix)
        string(APPEND symbols_dir "/${region}")
        string(APPEND patch_dir "/build/${region}")
        set(patch_config "${CMAKE_SOURCE_DIR}/patches.${region}.toml")
    endif()

    set(func_dir "${CMAKE_SOURCE_DIR}/RecompiledFuncs${suffix}")
    set(output_dir "${CMAKE_SOURCE_DIR}/RecompiledPatches${suffix}")
    file(GLOB func_sources CONFIGURE_DEPENDS "${func_dir}/*.c" "${func_dir}/*.cpp")
    if(NOT func_sources)
        message(FATAL_ERROR "Generate both regions first: python3 scripts/generate-code.py")
    endif()

    add_custom_target(PatchesBin${suffix}
        COMMAND ${CMAKE_COMMAND} -E env CC=${PATCHES_C_COMPILER} LD=${PATCHES_LD} make VERSION=${region}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/patches"
        BYPRODUCTS "${patch_dir}/patches.elf"
        VERBATIM)

    add_custom_command(OUTPUT
        "${output_dir}/patches.c"
        "${output_dir}/recomp_overlays.inl"
        "${output_dir}/funcs.h"
        "${patch_dir}/patches.bin"
        COMMAND ${CMAKE_COMMAND}
            -DRUN_EXECUTABLE=./N64Recomp
            -DRUN_ARGUMENTS=${patch_config}
            -DRUN_ATTEMPTS=${PATCH_RECOMP_ATTEMPTS}
            -DRUN_DELAY_SECONDS=2
            -DRUN_WORKING_DIRECTORY=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/RunWithRetries.cmake"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        DEPENDS "${patch_dir}/patches.elf" "${patch_config}"
            "${symbols_dir}/dump.toml" "${symbols_dir}/data_dump.toml"
        VERBATIM)

    add_custom_command(OUTPUT "${output_dir}/patches_bin.c" "${output_dir}/patches_bin.h"
        COMMAND file_to_c "${patch_dir}/patches.bin" mm_patches_bin
            "${output_dir}/patches_bin.c" "${output_dir}/patches_bin.h"
        DEPENDS "${patch_dir}/patches.bin" file_to_c
        VERBATIM)

    add_library(RecompiledFuncs${suffix} STATIC ${func_sources})
    add_library(PatchesLib${suffix} STATIC "${output_dir}/patches.c" "${output_dir}/patches_bin.c")
    add_dependencies(PatchesLib${suffix} PatchesBin${suffix})
    foreach(target RecompiledFuncs${suffix} PatchesLib${suffix})
        target_include_directories(${target} PRIVATE
            "${CMAKE_SOURCE_DIR}/include"
            "${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/ultramodern/include"
            "${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/librecomp/include"
            "${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/N64Recomp/include")
        if(NOT MSVC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target} PRIVATE
                -fno-strict-aliasing -Wno-unused-variable -Wno-implicit-function-declaration)
        endif()
    endforeach()

    # JP native names need a prefix to coexist with US names in the same executable.
    if(region STREQUAL "jp")
        set(symbol_header "${CMAKE_CURRENT_BINARY_DIR}/generated/jp_symbols.h")
        add_custom_command(OUTPUT "${symbol_header}"
            COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/scripts/namespace-region.py"
                --output "${symbol_header}" "${func_dir}" "${output_dir}"
            DEPENDS ${func_sources} "${output_dir}/patches.c" "${CMAKE_SOURCE_DIR}/scripts/namespace-region.py"
            VERBATIM)
        add_custom_target(JpSymbols DEPENDS "${symbol_header}")
        foreach(target RecompiledFuncs${suffix} PatchesLib${suffix})
            add_dependencies(${target} JpSymbols)
            if(MSVC)
                target_compile_options(${target} PRIVATE "/FI${symbol_header}")
            else()
                target_compile_options(${target} PRIVATE -include "${symbol_header}")
            endif()
        endforeach()
    endif()
endfunction()
