file(GLOB HEADERS "include/ultrainio/chain_info_api_plugin/*.hpp")
add_library( chain_info_api_plugin
             chain_info_api_plugin.cpp
             ${HEADERS} )

target_link_libraries( chain_info_api_plugin chain_plugin http_plugin appbase )
target_include_directories( chain_info_api_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
