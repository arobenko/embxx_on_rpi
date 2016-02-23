cmake_minimum_required (VERSION 2.8)

include(cmake/toolchain_handling.cmake)



project ("embxx_on_rpi")

set (EXTERNALS_DIR "${CMAKE_SOURCE_DIR}/external")
find_package (Git REQUIRED)
 
execute_process (
    COMMAND ${GIT_EXECUTABLE} submodule update --init
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
 
set (CMAKE_MODULE_PATH "${EXTERNALS_DIR}/embxx/cmake")         
include (AR_AllExtras)
    
embxx_add_cpp11_support ()
embxx_set_default_compiler_options ()

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
 
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

set (STARTUP_LIB_NAME "startup")
set (DEVICE_LIB_NAME "device")
set (STDLIB_STUB_LIB_NAME "stdlib_stub")

add_subdirectory (src)

print_toolchain_used() # from cmake/toolchain_handling.cmake
