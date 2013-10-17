function (lib_device)
    set (name "${DEVICE_LIB_NAME}")
    
    set (src 
        "${CMAKE_CURRENT_SOURCE_DIR}/Function.cpp")

    add_library(${name} STATIC ${src})
endfunction ()

#################################################################

lib_device()