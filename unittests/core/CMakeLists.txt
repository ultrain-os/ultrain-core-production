add_executable( message_test
        MessageTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( message_test ultrainio_core )

add_test(NAME message_test COMMAND message_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
