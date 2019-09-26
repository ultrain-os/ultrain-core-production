file(GLOB HEADERS "include/kcp/*.h")
add_library( ultrainio_kcp
    src/ikcp.c
    ${HEADERS})
target_include_directories( ultrainio_kcp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
