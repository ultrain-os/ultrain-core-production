### BlsVoterSet
add_executable( blsvoterset_test
        BlsVoterSetTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( blsvoterset_test ultrainio_core )

add_test(NAME blsvoterset_test COMMAND blsvoterset_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
