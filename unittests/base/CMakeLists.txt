add_executable( hex_test
        HexTest.cpp)

FIND_PACKAGE(Boost 1.67 REQUIRED COMPONENTS
        chrono
        unit_test_framework
        iostreams)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( hex_test ultrainio_base )

add_test(NAME hex_test COMMAND hex_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})