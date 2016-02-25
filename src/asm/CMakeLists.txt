
function (lib_startup)
    set (name "${STARTUP_LIB_NAME}")
    
    set (src 
        "${CMAKE_CURRENT_SOURCE_DIR}/startup.s")


    add_library(${name} STATIC ${src})
    
endfunction ()

#################################################################

# Make sure there are not "-D or -O" options
set (CMAKE_BUILD_TYPE "Debug")
embxx_add_asm_flags ("-march=armv6z")
enable_language (ASM)

lib_startup ()
