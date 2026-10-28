# The vcpkg toolchain and manifest pin Crashpad and its transitive dependencies.
find_package(crashpad CONFIG REQUIRED)
get_filename_component(SK1_CRASHPAD_PREFIX "${crashpad_DIR}/../.." ABSOLUTE)
find_program(SK1_CRASHPAD_HANDLER crashpad_handler.exe
    PATHS "${SK1_CRASHPAD_PREFIX}/tools/crashpad" NO_DEFAULT_PATH REQUIRED)

add_library(sk1_crash_reports STATIC "${CMAKE_CURRENT_LIST_DIR}/../src/main/crash_reports.cpp")
target_include_directories(sk1_crash_reports PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include")
target_include_directories(sk1_crash_reports PRIVATE "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(sk1_crash_reports PRIVATE crashpad::crashpad shell32 user32 advapi32 rpcrt4 winhttp version)

function(sk1_stage_crashpad target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SK1_CRASHPAD_HANDLER}" "$<TARGET_FILE_DIR:${target}>/crashpad_handler.exe"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../licenses/crashpad" "$<TARGET_FILE_DIR:${target}>/licenses/crashpad"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SK1_CRASHPAD_PREFIX}/share/crashpad/copyright"
            "$<TARGET_FILE_DIR:${target}>/licenses/crashpad/LICENSE"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SK1_CRASHPAD_PREFIX}/share/zlib/copyright"
            "$<TARGET_FILE_DIR:${target}>/licenses/crashpad/zlib-LICENSE"
        VERBATIM)
endfunction()
