function (lib_stdlib_stub)
    set (name "${STDLIB_STUB_LIB_NAME}")
    
    set (src 
        "${CMAKE_CURRENT_SOURCE_DIR}/stdlib_stub.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/placeholders.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/string.cpp")

    add_library(${name} STATIC ${src})
endfunction ()

#################################################################

lib_stdlib_stub()