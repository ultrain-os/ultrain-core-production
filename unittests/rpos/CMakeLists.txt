add_executable( rpos_unittest
        rpos_unittest_main.cpp)

target_link_libraries( rpos_unittest ultrainio_rpos )

add_test(NAME rpos_unittest COMMAND rpos_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})