file(GLOB HEADERS "include/log/*.h")
add_library( ultrainio_log
        src/Log.cpp
        ${HEADERS} )

FIND_PACKAGE(Boost 1.66 REQUIRED COMPONENTS
        system
        log
        thread
        filesystem)

target_link_libraries( ultrainio_log chainbase ${Boost_LIBRARIES} )
target_include_directories( ultrainio_log PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
