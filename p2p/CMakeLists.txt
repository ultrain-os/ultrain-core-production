file(GLOB HEADERS "include/p2p/*.h")
add_library( ultrainio_p2p
        src/NodeTable.cpp
        ${HEADERS} )


target_link_libraries( ultrainio_p2p ultrainio_core)
target_include_directories( ultrainio_p2p PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
