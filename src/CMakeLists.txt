
set (LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/raspberrypi.ld)

#################################################################

function (link_app tgt)

    set_target_properties (${tgt}
        PROPERTIES LINK_FLAGS "-Wl,-Map,kernel.map -T ${LINKER_SCRIPT}")
    
    add_custom_command (
        TARGET ${tgt}
        POST_BUILD
        COMMAND 
            ${CMAKE_OBJCOPY} $<TARGET_FILE:${tgt}> -O binary
                                        $<TARGET_FILE_DIR:${tgt}>/kernel.img
        COMMAND 
            ${CMAKE_OBJDUMP} -D -S $<TARGET_FILE:${tgt}> >
                                        $<TARGET_FILE_DIR:${tgt}>/kernel.list
        COMMAND 
            ${CMAKE_OBJDUMP} -t $<TARGET_FILE:${tgt}> >
                                        $<TARGET_FILE_DIR:${tgt}>/kernel.syms
        COMMAND 
            ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/image/${tgt}
        COMMAND 
            ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${tgt}>/kernel.img ${CMAKE_BINARY_DIR}/image/${tgt})
                                        
endfunction ()

#################################################################

add_subdirectory (asm)

embxx_add_c_cxx_flags ("-march=armv6z")

if (NOT NO_TEST_CPP)
    add_subdirectory (test_cpp)
endif ()


embxx_add_c_cxx_flags ("-fno-threadsafe-statics")

include_directories (
    ${EXTERNALS_DIR}/embxx
    ${CMAKE_CURRENT_SOURCE_DIR})

embxx_disable_exceptions ()
embxx_disable_rtti ()
embxx_disable_stdlib ()

add_subdirectory (device)
add_subdirectory (stdlib)
add_subdirectory (app)
