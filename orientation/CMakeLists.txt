set(SOURCES orientation.c imu.c gps.c filter.c)
get_filename_component(LIBNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_library(${LIBNAME} SHARED "${SOURCES}")
