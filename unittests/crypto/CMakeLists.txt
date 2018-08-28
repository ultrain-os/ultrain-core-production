add_executable( key_test
        KeyTest.cpp)

target_link_libraries( key_test ultrainio_rpos )

add_test(NAME key_test COMMAND key_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})