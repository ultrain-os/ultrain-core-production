### BlsVoterSet
add_executable( blsvoterset_test
        BlsVoterSetTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( blsvoterset_test ultrainio_lightclient )

add_test(NAME blsvoterset_test COMMAND blsvoterset_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### CommitteeSet
add_executable( committeeset_test_suite
        CommitteeSetTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( committeeset_test_suite ultrainio_lightclient )

add_test(NAME committeeset_test_suite COMMAND committeeset_test_suite WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
