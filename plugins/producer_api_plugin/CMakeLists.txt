file(GLOB HEADERS "include/ultrainio/producer_api_plugin/*.hpp")
add_library( producer_api_plugin
             producer_api_plugin.cpp
             ${HEADERS} )

target_link_libraries( producer_api_plugin producer_uranus_plugin http_plugin appbase )
target_include_directories( producer_api_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
