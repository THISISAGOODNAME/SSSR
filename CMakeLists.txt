cmake_minimum_required(VERSION 3.20)
project(SSSR)

#add_executable(SSSR main.cpp)

set(project_root "${CMAKE_CURRENT_SOURCE_DIR}")
set(assets_path "${project_root}/assets/")
add_definitions(-DASSETS_PATH="${assets_path}")

list(PREPEND CMAKE_MODULE_PATH "${project_root}/cmake")
include(cmake_settings)
include(compiler_settings)

add_subdirectory(3rdparty/FlyCube)

add_subdirectory(src/01Triangle)

include(3rdparty/imgui)
include(3rdparty/IconFontCppHeaders)

add_subdirectory(src/SSSR)

option(SSSR_BUILD_TOOLS "Whether build SSSR tools" ON)

if (${SSSR_BUILD_TOOLS})
    add_subdirectory(src/Tools)
endif()
