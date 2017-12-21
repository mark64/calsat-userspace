set(SOURCES i2cctl.c)
get_filename_component(LIBNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_library(${LIBNAME} SHARED "${SOURCES}")
