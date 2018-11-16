add_executable( wssultrain main.cpp )
target_link_libraries( wssultrain
        PRIVATE appbase
        PRIVATE http_plugin
        PRIVATE -Wl,${whole_archive_flag} sync_net_plugin                 -Wl,${no_whole_archive_flag}
        PRIVATE -Wl,${whole_archive_flag} sync_net_api_plugin             -Wl,${no_whole_archive_flag}
        PRIVATE fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
