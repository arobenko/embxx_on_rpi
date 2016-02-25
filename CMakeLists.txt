cmake_minimum_required (VERSION 2.8)


if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})
  message("==> Loading \${CMAKE_TOOLCHAIN_FILE} == ${CMAKE_TOOLCHAIN_FILE}")

else()

  set (CMAKE_SYSTEM_NAME "Generic")

  if ("${CROSS_COMPILE}" STREQUAL "")
    set (CROSS_COMPILE "arm-none-eabi-")
  endif()

  if (NOT    CMAKE_VERSION VERSION_LESS 3.5.0)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
    set(CMAKE_C_COMPILER   "${CROSS_COMPILE}gcc")
    set(CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++")
  else()
    include(CMakeForceCompiler)
    cmake_force_c_compiler(  "${CROSS_COMPILE}gcc" GNU)
    cmake_force_cxx_compiler("${CROSS_COMPILE}g++" GNU)
  endif()

endif()


function(print_toolchain_used)
  message("## assembler used: (this is displayed above this line, on first cmake run only)")
  message("## using \${CMAKE_C_COMPILER}                  == ${CMAKE_C_COMPILER}")
  message("## using \${CMAKE_CXX_COMPILER}                == ${CMAKE_CXX_COMPILER}")
  message("## using \${CMAKE_AR}                          == ${CMAKE_AR}")
  message("## using \${CMAKE_RANLIB}                      == ${CMAKE_RANLIB}")
  message("## using \${CMAKE_OBJCOPY}                     == ${CMAKE_OBJCOPY}")
  message("## using \${CMAKE_OBJDUMP}                     == ${CMAKE_OBJDUMP}")
endfunction()


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

print_toolchain_used()
