file(GLOB SOURCES "*.cpp" "*.c" "*.cc")
get_filename_component(LIBNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_library(${LIBNAME} SHARED "${SOURCES}")
