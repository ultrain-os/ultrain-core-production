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

### VoterSet

add_executable( voterset_unittest
        VoterSetTest.cpp)

target_link_libraries( voterset_unittest ultrainio_rpos )

add_test(NAME voterset_unittest COMMAND voterset_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### EvilBlsDetector

add_executable( evilblsdetector_unittest
        EvilBlsDetectorTest.cpp)

target_link_libraries( evilblsdetector_unittest ultrainio_rpos )

add_test(NAME evilblsdetector_unittest COMMAND evilblsdetector_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

### EvidenceMultiSign

add_executable( evidencemultisign_unittest
        EvidenceMultiSignTest.cpp)

target_link_libraries( evidencemultisign_unittest ultrainio_rpos )

add_test(NAME evidencemultisign_unittest COMMAND evidencemultisign_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
