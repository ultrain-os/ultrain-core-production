add_executable( crypto_unittest
        crypto_unittest_main.cpp)

target_link_libraries( crypto_unittest ultrainio_rpos )

add_test(NAME crypto_unittest COMMAND crypto_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})