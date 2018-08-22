file(GLOB HEADERS "include/p2p/*.h")
add_library( ultrainio_p2p
        src/NetManager.cpp
        ${HEADERS} )

FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS
        system
        thread
        filesystem)

target_link_libraries( ultrainio_p2p ${Boost_LIBRARIES} )
target_include_directories( ultrainio_p2p PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
