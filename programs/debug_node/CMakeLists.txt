add_executable( debug_node main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling debug_node with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

target_link_libraries( debug_node
                       PRIVATE ultrain_app ultrain_producer ultrain_debug_producer ultrain_chain ultrain_egenesis_full fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )

install( TARGETS
   debug_node

   RUNTIME DESTINATION usr/bin
   LIBRARY DESTINATION usr/lib
   ARCHIVE DESTINATION usr/lib
)
