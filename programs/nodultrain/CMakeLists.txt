add_executable( nodultrain main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling nodultrain with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../.git)
  find_package(Git)
  if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short=8 HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../.."
      OUTPUT_VARIABLE "nodultrain_BUILD_VERSION"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Git commit revision: ${nodultrain_BUILD_VERSION}")
  else()
    set(nodultrain_BUILD_VERSION 0)
  endif()
else()
  set(nodultrain_BUILD_VERSION 0)
endif()

configure_file(config.hpp.in config.hpp ESCAPE_QUOTES)

target_include_directories(nodultrain PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

if(UNIX)
  if(APPLE)
    set(whole_archive_flag "-force_load")
    set(no_whole_archive_flag "")
  else()
    set(whole_archive_flag "--whole-archive")
    set(no_whole_archive_flag "--no-whole-archive")
  endif()
else()
  set(whole_archive_flag "--whole-archive")
  set(no_whole_archive_flag "--no-whole-archive")
endif()

target_link_libraries( nodultrain
        PRIVATE appbase
        PRIVATE -Wl,${whole_archive_flag} chain_api_plugin           -Wl,${no_whole_archive_flag}
        PRIVATE -Wl,${whole_archive_flag} chain_info_api_plugin      -Wl,${no_whole_archive_flag}
        PRIVATE -Wl,${whole_archive_flag} wallet_api_plugin          -Wl,${no_whole_archive_flag}
        PRIVATE -Wl,${whole_archive_flag} net_plugin                 -Wl,${no_whole_archive_flag}
        PRIVATE -Wl,${whole_archive_flag} mongo_db_plugin            -Wl,${no_whole_archive_flag}
        PRIVATE -Wl,${whole_archive_flag} monitor_plugin             -Wl,${no_whole_archive_flag}
        PRIVATE -Wl,${whole_archive_flag} txn_test_gen_plugin        -Wl,${no_whole_archive_flag}
        PRIVATE -Wl,${whole_archive_flag} producer_api_plugin        -Wl,${no_whole_archive_flag}
        PRIVATE chain_plugin http_plugin producer_uranus_plugin http_client_plugin
        PRIVATE ultrainio_chain fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )

install( TARGETS
   nodultrain

   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/log/ultrainio
        DIRECTORY_PERMISSIONS OWNER_READ
                              OWNER_WRITE
                              OWNER_EXECUTE
                              GROUP_READ
                              GROUP_WRITE
                              GROUP_EXECUTE
                              WORLD_READ
                              WORLD_EXECUTE
)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/ultrainio
        DIRECTORY_PERMISSIONS OWNER_READ
                              OWNER_WRITE
                              OWNER_EXECUTE
                              GROUP_READ
                              GROUP_WRITE
                              GROUP_EXECUTE
                              WORLD_READ
                              WORLD_EXECUTE
			)

mas_sign(nodultrain)
