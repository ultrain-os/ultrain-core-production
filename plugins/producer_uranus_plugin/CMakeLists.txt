file(GLOB HEADERS "include/ultrainio/producer_uranus_plugin/*.hpp")
add_library( producer_uranus_plugin
             producer_uranus_plugin.cpp
             connect.cpp
             node.cpp
             pktmanage.cpp
             security.cpp
             ${HEADERS} )

include_directories( "../../externals/boringssl/include" )
target_link_libraries( producer_uranus_plugin  appbase fc fiat crypto)
target_include_directories( producer_uranus_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )


