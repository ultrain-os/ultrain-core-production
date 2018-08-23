file(GLOB HEADERS "include/consensus/*.h")
add_library( ultrainio_uranus
        src/MessageBuilder.cpp
        src/MessageManager.cpp
        src/Node.cpp
        src/UranusController.cpp
        src/VoterSystem.cpp
        ${HEADERS} )

target_link_libraries( ultrainio_uranus fc appbase net_plugin ultrainio_core ultrainio_crypto ultrainio_log )
target_include_directories( ultrainio_uranus PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
