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

### StakeVote
#add_executable( votersystem_unittest
#        VoterSystemTest.cpp)

#target_link_libraries( votersystem_unittest ultrainio_rpos )

#add_test(NAME votersystem_unittest COMMAND votersystem_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### MessageManager
add_executable( messagemanager_unittest
        MessageManagerTest.cpp)

target_link_libraries( messagemanager_unittest ultrainio_rpos )

add_test(NAME messagemanager_unittest COMMAND messagemanager_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### PhaseMessage
add_executable( phasemessage_unittest
        PhaseMessageTest.cpp)

target_link_libraries( phasemessage_unittest ultrainio_rpos )

add_test(NAME phasemessage_unittest COMMAND phasemessage_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### BlockMessage
add_executable( blockmessage_unittest
        BlockMessageTest.cpp)

target_link_libraries( blockmessage_unittest ultrainio_rpos )

add_test(NAME blockmessage_unittest COMMAND blockmessage_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})