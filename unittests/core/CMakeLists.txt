### BlsVoterSet
add_executable( blsvoterset_test
        BlsVoterSetTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( blsvoterset_test ultrainio_core )

add_test(NAME blsvoterset_test COMMAND blsvoterset_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### EvidenceMultiPropose

add_executable( multiproposeevidence_unittest
        MultiProposeEvidenceTest.cpp)

target_link_libraries( multiproposeevidence_unittest ultrainio_core )

add_test(NAME multiproposeevidence_unittest COMMAND multiproposeevidence_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### EvidenceMultiVote

add_executable( multivoteevidence_unittest
        MultiVoteEvidenceTest.cpp)

target_link_libraries( multivoteevidence_unittest ultrainio_core )

add_test(NAME multivoteevidence_unittest COMMAND multivoteevidence_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
