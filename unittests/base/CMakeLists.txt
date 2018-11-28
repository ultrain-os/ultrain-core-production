add_executable( hex_test
        HexTest.cpp)

target_link_libraries( hex_test ultrainio_base )

add_test(NAME hex_test COMMAND hex_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})