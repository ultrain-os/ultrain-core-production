add_executable( test_cypher_suites test_cypher_suites.cpp )
target_link_libraries( test_cypher_suites fc )

add_test(NAME test_cypher_suites COMMAND vendor/libraries/fc/test/crypto/test_cypher_suites WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")