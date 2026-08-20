cmake_minimum_required(VERSION 3.20)

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
file(COPY "${PROJECT_ROOT}/lib/recomp-theme/assets/" DESTINATION "${OUTPUT_DIR}")
file(COPY "${PROJECT_ROOT}/assets/" DESTINATION "${OUTPUT_DIR}")
