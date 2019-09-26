file(GLOB HEADERS "include/p2p/*.h")
add_library( ultrainio_p2p
    src/NodeTable.cpp
    src/UPnP.cpp
    ${HEADERS} )
include_directories( "${PROJECT_SOURCE_DIR}/vendor/miniupnpc" )
target_link_libraries( ultrainio_p2p ultrainio_core miniupnpc-private libminiupnpc-static ultrainio_kcp)
target_include_directories( ultrainio_p2p PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
