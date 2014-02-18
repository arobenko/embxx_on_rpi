function (bin_test_statics)
    set (name "03_test_statics")
    
    set (src 
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SomeObj.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SomeObj2.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/stub.cpp")

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
embxx_add_c_cxx_flags ("-fno-threadsafe-statics")

bin_test_statics()
