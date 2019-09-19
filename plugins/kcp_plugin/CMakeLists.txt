file(GLOB HEADERS "include/ultrainio/kcp_plugin/*.hpp" )
add_library( kcp_plugin
             kcp_plugin.cpp
             ${HEADERS} )

target_link_libraries( kcp_plugin chain_plugin producer_rpos_plugin appbase fc ultrainio_p2p )
target_include_directories( kcp_plugin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../chain_interface/include  "${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/appbase/include")
