file(GLOB HEADERS "include/ultrainio/sync_net_api_plugin/*.hpp")
add_library( sync_net_api_plugin
             sync_net_api_plugin.cpp
             ${HEADERS} )

target_link_libraries( sync_net_api_plugin sync_net_plugin http_plugin appbase )
target_include_directories( sync_net_api_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
