cmake_minimum_required(VERSION 3.27)
project(media_player)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavformat
        libavcodec
        libswresample
        libavutil
)

pkg_check_modules(OTHER REQUIRED IMPORTED_TARGET
        SDL2
        spdlog
)

add_subdirectory(core)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
        PkgConfig::FFMPEG
        PkgConfig::OTHER
        core)