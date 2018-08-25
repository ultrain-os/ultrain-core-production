file(GLOB HEADERS "include/rpos/*.h")
add_library( ultrainio_rpos
        src/BlockMessage.cpp
        src/MessageBuilder.cpp
        src/MessageManager.cpp
        src/Node.cpp
        src/PhaseMessage.cpp
        src/Proof.cpp
        src/Seed.cpp
        src/UranusController.cpp
        src/VoterSystem.cpp
        src/Vrf.cpp
        ${HEADERS} )

target_link_libraries( ultrainio_rpos fc appbase net_plugin ultrainio_core ultrainio_crypto ultrainio_log )
target_include_directories( ultrainio_rpos PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
