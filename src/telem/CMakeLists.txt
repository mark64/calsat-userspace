set(SOURCES
    pwr.cpp
)
get_filename_component(DIRNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_executable(${DIRNAME}d main.cpp ${SOURCES})

if(BUILD_TESTING)
    add_executable(${DIRNAME}d-test test.cpp ${SOURCES})
    target_link_libraries(${DIRNAME}d-test drivers UnitTest++)
    add_test(${DIRNAME}d-test ${DIRNAME}d-test)
endif()
