aux_source_directory(. DIR_SRCS)


add_library(core ${DIR_SRCS})

target_link_libraries(core
        PkgConfig::FFMPEG
        PkgConfig::OTHER)