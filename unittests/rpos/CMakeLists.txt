### Signer/Validator
add_executable( signer_validator_unittest
        SignerValidatorTest.cpp)

target_link_libraries( signer_validator_unittest ultrainio_rpos )

add_test(NAME signer_validator_unittest COMMAND signer_validator_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### Proof
add_executable( proof_unittest
        ProofTest.cpp)

target_link_libraries( proof_unittest ultrainio_rpos )

add_test(NAME proof_unittest COMMAND proof_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### FisherYates
add_executable( fisheryates_unittest
        FisherYatesTest.cpp)

target_link_libraries( fisheryates_unittest ultrainio_rpos )

add_test(NAME fisheryates_unittest COMMAND fisheryates_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### RoleRandom
add_executable( rolerandom_unittest
        RoleRandomTest.cpp)

target_link_libraries( rolerandom_unittest ultrainio_rpos )

add_test(NAME rolerandom_unittest COMMAND rolerandom_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### RoleSelection
add_executable( roleselection_unittest
        RoleSelectionTest.cpp)

target_link_libraries( roleselection_unittest ultrainio_rpos )

add_test(NAME roleselection_unittest COMMAND roleselection_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### PunishMgr
add_executable( punishmgr_unittest
        PunishMgrTest.cpp)

target_link_libraries( punishmgr_unittest ultrainio_rpos )

add_test(NAME punishmgr_unittest COMMAND punishmgr_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### BlsVoterSet
add_executable( blsvoterset_test
        BlsVoterSetTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( blsvoterset_test ultrainio_rpos )

add_test(NAME blsvoterset_test COMMAND blsvoterset_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### CommitteeSet
add_executable( committeeset_test_suite
        CommitteeSetTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( committeeset_test_suite ultrainio_rpos )

add_test(NAME committeeset_test_suite COMMAND committeeset_test_suite WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
