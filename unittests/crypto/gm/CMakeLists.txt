FIND_PACKAGE(Boost 1.67 REQUIRED COMPONENTS
        chrono
        unit_test_framework
        iostreams)


# gm
add_executable( gm_unittest
        GmTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( gm_unittest ultrainio_gm)

add_test(NAME gm_unittest COMMAND gm_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
