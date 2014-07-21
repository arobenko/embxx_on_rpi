function (bin_test_rtti)
    set (name "test_cpp_rtti")
    
    set (src 
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/stub.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SomeClass.cpp")

    set (link
        ${STARTUP_LIB_NAME})

    add_executable(${name} ${src})
    target_link_libraries (${name} ${link})
    link_app (${name})
endfunction ()

#################################################################

embxx_disable_exceptions ()
#embxx_disable_rtti ()

bin_test_rtti()
