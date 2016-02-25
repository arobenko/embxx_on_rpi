cmake_minimum_required (VERSION 2.8)

if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message("==> Loading \${CMAKE_TOOLCHAIN_FILE} == ${CMAKE_TOOLCHAIN_FILE}")
else()

    set (CMAKE_SYSTEM_NAME "Generic")
    if ("${CROSS_COMPILE}" STREQUAL "")
        set (CROSS_COMPILE "arm-none-eabi-")
    endif()

    include(CMakeForceCompiler)
    cmake_force_c_compiler(  "${CROSS_COMPILE}gcc" GNU)
    cmake_force_cxx_compiler("${CROSS_COMPILE}g++" GNU)

endif()

#########

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

message(STATUS "Found C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Found C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Found ar: ${CMAKE_AR}")
message(STATUS "Found ranlib: ${CMAKE_RANLIB}")
message(STATUS "Found objcopy: ${CMAKE_OBJCOPY}")
message(STATUS "Found objdump: ${CMAKE_OBJDUMP}")
