set(SOURCES
    i2cctl.cpp
    ina3221.cpp
)
get_filename_component(DIRNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_library(${DIRNAME} SHARED ${SOURCES})

if(BUILD_TESTING)
    add_executable(${DIRNAME}-test test.cpp ${SOURCES})
    target_link_libraries(${DIRNAME}-test UnitTest++)
    add_test(${DIRNAME}-test ${DIRNAME}-test)
endif()
