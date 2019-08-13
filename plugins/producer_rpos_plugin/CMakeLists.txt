file(GLOB HEADERS "include/ultrainio/producer_rpos_plugin/*.hpp")
add_library( producer_rpos_plugin
             producer_rpos_plugin.cpp
             ${HEADERS} )

target_link_libraries( producer_rpos_plugin chain_plugin http_client_plugin ultrainio_chain ultrain_utilities net_plugin appbase fc ultrainio_rpos ultrainio_core )
target_include_directories( producer_rpos_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
