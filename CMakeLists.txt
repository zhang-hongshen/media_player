cmake_minimum_required(VERSION 3.27)
project(media_player)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil
        SDL2
        spdlog
)

add_executable(${PROJECT_NAME} main.cpp
        avframe_queue.cpp
        avframe_queue.h
        avpacket_queue.cpp
        avpacket_queue.h
        utils/thread.cpp
        utils/thread.h
        demux.cpp
        demux.h
        utils/queue.h
        decoder.cpp
        decoder.h
        video_output.cpp
        video_output.h
        audio_output.cpp
        audio_output.h
        avsync.cpp
        avsync.h
)

target_link_libraries(${PROJECT_NAME} PkgConfig::LIBAV)