file(GLOB HEADERS "include/ultrainio/monitor_plugin/*.hpp")
add_library( monitor_plugin
             monitor_plugin.cpp
             ${HEADERS} )

target_link_libraries( monitor_plugin appbase fc ultrainio_uranus)
target_include_directories( monitor_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
