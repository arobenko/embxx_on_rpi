cmake_minimum_required (VERSION 2.8)
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

set (CMAKE_SYSTEM_NAME "Generic")               ## possibly this needs to be in the else() branch below (?)

set(toolchain   ""      CACHE FILEPATH     "")  ## -Dtoolchain=/my/path/to/toolchain_file
if (toolchain AND EXISTS ${toolchain})
  message("==> Including toolchain_file ${toolchain}")
  include(${toolchain})
else()

  if ("${CROSS_COMPILE}" STREQUAL "")
    set (CROSS_COMPILE "arm-none-eabi-")
  endif()

  set (CMAKE_C_COMPILER   "${CROSS_COMPILE}gcc")
  set (CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++")
  set (CMAKE_ASM_COMPILER "${CROSS_COMPILE}as")
  set (CMAKE_AR           "${CROSS_COMPILE}ar")
  set (CMAKE_RANLIB       "${CROSS_COMPILE}ranlib")

endif()

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
 
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

set (STARTUP_LIB_NAME "startup")
set (DEVICE_LIB_NAME "device")
set (STDLIB_STUB_LIB_NAME "stdlib_stub")

add_subdirectory (src)

