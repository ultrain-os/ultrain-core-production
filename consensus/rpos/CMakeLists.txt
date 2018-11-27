file(GLOB HEADERS "include/rpos/*.h")
add_library( ultrainio_rpos
        src/BlockMsg.cpp
        src/Config.cpp
        src/FisherYates.cpp
        src/Genesis.cpp
        src/MsgBuilder.cpp
        src/MsgMgr.cpp
        src/Node.cpp
        src/NodeInfo.cpp
        src/PhaseMsg.cpp
        src/Proof.cpp
        src/RoleRandom.cpp
        src/RoleSelection.cpp
        src/Scheduler.cpp
        src/Seed.cpp
        src/StakeVoteBase.cpp
        src/StakeVoteFactory.cpp
        src/StakeVoteRandom.cpp
        src/Vrf.cpp
        ${HEADERS} )

target_link_libraries( ultrainio_rpos appbase net_plugin ultrainio_core ultrainio_crypto )
target_include_directories( ultrainio_rpos PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
