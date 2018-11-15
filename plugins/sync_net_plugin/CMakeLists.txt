file(GLOB HEADERS "include/ultrainio/sync_net_plugin/*.hpp" )
add_library( sync_net_plugin
            sync_net_plugin.cpp
             ${HEADERS} )

target_link_libraries( sync_net_plugin appbase fc )
target_include_directories( sync_net_plugin
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        "${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/appbase/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/libraries/chain/include")
