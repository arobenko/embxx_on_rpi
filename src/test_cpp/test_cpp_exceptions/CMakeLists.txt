function (bin_test_exceptions)
    set (name "test_cpp_exceptions")
    
    set (src 
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/stub.cpp")

    set (link
        ${STARTUP_LIB_NAME})

    add_executable(${name} ${src})
    target_link_libraries (${name} ${link})
    link_app (${name})
endfunction ()

#################################################################

embxx_disable_exceptions ()
#embxx_disable_stdlib()

bin_test_exceptions()
