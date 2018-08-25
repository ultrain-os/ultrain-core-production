file(GLOB HEADERS "include/ultrainio/producer_uranus_plugin/*.hpp")
add_library( producer_uranus_plugin
             producer_uranus_plugin.cpp
             ${HEADERS} )

target_link_libraries( producer_uranus_plugin chain_plugin http_client_plugin ultrainio_chain ultrain_utilities net_plugin appbase fc ultrainio_log ultrainio_rpos ultrainio_core )
target_include_directories( producer_uranus_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )


