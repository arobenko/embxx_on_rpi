function (bin_test_tag_dispatch)
    set (name "test_cpp_tag_dispatch")
    
    set (src 
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tag.cpp")

    set (link
        ${STARTUP_LIB_NAME})

    add_executable(${name} ${src})
    target_link_libraries (${name} ${link})
    link_app (${name})
endfunction ()

#################################################################

embxx_disable_exceptions ()
embxx_disable_rtti ()
embxx_disable_stdlib ()

bin_test_tag_dispatch()
