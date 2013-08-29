cmake_minimum_required (VERSION 2.8)
project ("rpi-embxx")

set (EXTERNALS_DIR "${CMAKE_SOURCE_DIR}/external")
find_package (Git REQUIRED)
 
execute_process (
    COMMAND ${GIT_EXECUTABLE} submodule update --init
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
 
set (CMAKE_MODULE_PATH "${EXTERNALS_DIR}/embxx/cmake")         
include (AR_AllExtras)
    
ar_add_cpp11_support ()
ar_set_default_compiler_options ()

set (CMAKE_SYSTEM_NAME "RPi")
set (CROSS_COMPILE_PREFIX "arm-none-eabi-")
set (CMAKE_C_COMPILER "${CROSS_COMPILE_PREFIX}gcc")
set (CMAKE_CXX_COMPILER "${CROSS_COMPILE_PREFIX}g++")
set (CMAKE_ASM_COMPILER "${CROSS_COMPILE_PREFIX}as")

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
 
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

set (STARTUP_LIB_NAME "startup")
set (DEVICE_LIB_NAME "device")

add_subdirectory (src)

