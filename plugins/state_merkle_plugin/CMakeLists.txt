file(GLOB HEADERS "include/ultrainio/state_merkle_plugin/*.hpp")
add_library( state_merkle_plugin
             state_merkle_plugin.cpp
             merkle_file_manager.cpp
             ${HEADERS} )

target_link_libraries( state_merkle_plugin chain_plugin state_history_plugin appbase )
target_include_directories( state_merkle_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
