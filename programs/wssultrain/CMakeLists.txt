add_executable( wssultrain main.cpp )

find_package( Gperftools QUIET )
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../.git)
  find_package(Git)
  if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short=8 HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../.."
      OUTPUT_VARIABLE "WSS_BUILD_VERSION"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Git commit revision: ${WSS_BUILD_VERSION}")
  else()
    set(WSS_BUILD_VERSION 0)
  endif()
else()
  set(WSS_BUILD_VERSION 0)
endif()

configure_file(config.hpp.in config.hpp ESCAPE_QUOTES)

target_include_directories(wssultrain PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries( wssultrain
        PRIVATE appbase
        PRIVATE http_plugin
        PRIVATE -Wl,${whole_archive_flag} sync_net_plugin                 -Wl,${no_whole_archive_flag}
        PRIVATE -Wl,${whole_archive_flag} sync_net_api_plugin             -Wl,${no_whole_archive_flag}
        PRIVATE fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
