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




function(print_toolchain_used)
  message("## assembler used: (this is displayed above this line, on first cmake run only)")
  message("## using \${CMAKE_C_COMPILER}                  == ${CMAKE_C_COMPILER}")
  message("## using \${CMAKE_CXX_COMPILER}                == ${CMAKE_CXX_COMPILER}")
  message("## using \${CMAKE_AR}                          == ${CMAKE_AR}")
  message("## using \${CMAKE_RANLIB}                      == ${CMAKE_RANLIB}")
  message("## using \${CMAKE_OBJCOPY}                     == ${CMAKE_OBJCOPY}")
  message("## using \${CMAKE_OBJDUMP}                     == ${CMAKE_OBJDUMP}")
endfunction()
