file(GLOB HEADERS "include/ultrainio/chain_plugin/*.hpp")
add_library( chain_plugin
             chain_plugin.cpp
             ${HEADERS} )

target_link_libraries( chain_plugin ultrainio_lightclient ultrainio_chain appbase )
target_include_directories( chain_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/../chain_interface/include" "${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/appbase/include")
