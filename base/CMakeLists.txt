file(GLOB HEADERS "include/base/*.h")
add_library( ultrainio_base
        src/Hex.cpp
        src/StringUtils.cpp
        ${HEADERS} )

target_link_libraries( ultrainio_base )
target_include_directories( ultrainio_base PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
