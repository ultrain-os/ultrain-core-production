file(GLOB HEADERS "include/core/*.h")
add_library( ultrainio_core
             src/BlsVoterSet.cpp
             src/EchoBlsEvidence.cpp
             src/Evidence.cpp
             src/EvidenceFactory.cpp
             src/ExtType.cpp
             src/Message.cpp
             src/MultiProposeEvidence.cpp
             src/MultiVoteEvidence.cpp
             src/NullEvidence.cpp
             src/SerializedEchoMsg.cpp
             src/utils.cpp
        ${HEADERS} )

target_link_libraries( ultrainio_core ultrainio_chain ultrainio_crypto )
target_include_directories( ultrainio_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
