set (LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/raspberrypi.ld)

#################################################################

function (link_app tgt)

    set_target_properties (${tgt}
        PROPERTIES LINK_FLAGS "-Wl,-Map,kernel.map -T ${LINKER_SCRIPT}")
    
    add_custom_command (
        TARGET ${tgt}
        POST_BUILD
        COMMAND 
            ${CROSS_COMPILE}objcopy $<TARGET_FILE:${tgt}> -O binary 
                                        $<TARGET_FILE_DIR:${tgt}>/kernel.img
        COMMAND 
            ${CROSS_COMPILE}objdump -D -S $<TARGET_FILE:${tgt}> > 
                                        $<TARGET_FILE_DIR:${tgt}>/kernel.list
        COMMAND 
            ${CROSS_COMPILE}objdump -t $<TARGET_FILE:${tgt}> > 
                                        $<TARGET_FILE_DIR:${tgt}>/kernel.syms
        COMMAND 
            ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/image/${tgt}
        COMMAND 
            ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${tgt}>/kernel.img ${CMAKE_BINARY_DIR}/image/${tgt})
                                        
endfunction ()

#################################################################

add_subdirectory (asm)

ar_add_c_cxx_flags ("-march=armv6z")
ar_add_c_cxx_flags ("-fno-threadsafe-statics")

include_directories (
    ${EXTERNALS_DIR}/embxx
    ${CMAKE_CURRENT_SOURCE_DIR})

ar_disable_exceptions ()
ar_disable_rtti ()
ar_disable_stdlib ()

add_subdirectory (device)
add_subdirectory (stdlib)
add_subdirectory (app)
