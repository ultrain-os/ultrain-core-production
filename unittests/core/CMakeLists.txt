### BlsVoterSet
add_executable( blsvoterset_test
        BlsVoterSetTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( blsvoterset_test ultrainio_core )

add_test(NAME blsvoterset_test COMMAND blsvoterset_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### EvidenceMultiSign

add_executable( multisignevidence_unittest
        MultiSignEvidenceTest.cpp)

target_link_libraries( multisignevidence_unittest ultrainio_core )

add_test(NAME multisignevidence_unittest COMMAND multisignevidence_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
