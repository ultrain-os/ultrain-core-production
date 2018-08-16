file(GLOB HEADERS "include/ultrainio/monitor_api_plugin/*.hpp")
add_library( monitor_api_plugin
             monitor_api_plugin.cpp
             ${HEADERS} )

target_link_libraries( monitor_api_plugin monitor_plugin producer_uranus_plugin http_plugin appbase)
target_include_directories( monitor_api_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
