function (bin_test_simple)
    set (name "01_test_simple")
    
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

bin_test_simple()
