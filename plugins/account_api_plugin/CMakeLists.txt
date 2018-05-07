file( GLOB HEADERS "include/ultrainio/account_api_plugin/*.hpp" )
add_library( account_api_plugin
        account_api_plugin.cpp
             ${HEADERS} )

target_link_libraries( account_api_plugin account_history_plugin wallet_plugin chain_plugin http_plugin appbase fc)
target_include_directories( account_api_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
target_include_directories( account_api_plugin PUBLIC ${CMAKE_BINARY_DIR}/contracts )

#add_dependencies( account_api_plugin )