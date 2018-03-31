add_custom_target(listtoolchains
    COMMAND echo To build using one of the toolchains below,
    run \"cmake -DCMAKE_TOOCHAIN_FILE=<file-path> ..\" in
    the build directory.
    &&
    ls -1 ${CMAKE_SOURCE_DIR}/cmake/toolchains/
)
