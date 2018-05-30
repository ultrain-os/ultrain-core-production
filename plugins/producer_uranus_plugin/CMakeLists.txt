file(GLOB HEADERS "include/ultrainio/producer_uranus_plugin/*.hpp")
add_library( producer_uranus_plugin
             producer_uranus_plugin.cpp
             connect.cpp
             log.cpp
             node.cpp
             pktmanage.cpp
             security.cpp
             ${HEADERS} )

FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS
        system
        chrono
        log
        iostreams
        thread
        filesystem)

include_directories( "../../externals/boringssl/include" )
target_link_libraries( producer_uranus_plugin  appbase fc fiat crypto ${Boost_LIBRARIES})
target_include_directories( producer_uranus_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )


