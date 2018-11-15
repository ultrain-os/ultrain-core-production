add_executable( wssultrain main.cpp )
target_link_libraries( wssultrain
        PRIVATE appbase
        PRIVATE sync_net_plugin
        PRIVATE http_plugin
        PRIVATE fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
