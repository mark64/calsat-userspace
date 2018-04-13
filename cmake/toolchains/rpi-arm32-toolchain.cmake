# this allows the raspberry pi 0/1/2 toolchain to be setup and placed in
#   the build directory
# sourced from the cmake wiki, located at the following URL
#   as of this writing on 3/17/17
# https://cmake.org/Wiki/CMake_Cross_Compiling
# by Mark Hill
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)
SET(TOOLCHAIN_REPO_URL https://github.com/raspberrypi/tools)
SET(TOOLCHAIN_REPO_DIR ${CMAKE_BUILD_DIR}/tools)
SET(TOOLCHAIN_TUPLE arm-bcm2708harhp-linux-gnueabi)
SET(TOOLCHAIN_ROOT_DIR ${TOOLCHAIN_REPO_DIR}/arm-bcm2708/${TOOLCHAIN_TUPLE}/${TOOLCHAIN_TUPLE})
SET(CMAKE_C_COMPILER $TOOLCHAIN_ROOT_DIR/bin/gcc)
SET(CMAKE_FIND_ROOT_PATH $TOOLCHAIN_ROOT_DIR/sysroot)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
if(EXISTS ${CMAKE_C_COMPILER})
    message(STATUS "coudn't find rpi0 gcc. Cloning from ${TOOLCHAIN_REPO}")
	execute_process(git clone --depth=1 ${TOOLCHAIN_REPO_URL} ${TOOLCHAIN_REPO_DIR})
endif()
