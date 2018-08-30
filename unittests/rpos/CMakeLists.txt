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

### VoterSystem
#add_executable( votersystem_unittest
#        VoterSystemTest.cpp)

#target_link_libraries( votersystem_unittest ultrainio_rpos )

#add_test(NAME votersystem_unittest COMMAND votersystem_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})