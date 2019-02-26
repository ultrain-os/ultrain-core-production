file(GLOB HEADERS "include/lightclient/*.h")

add_library( ultrainio_lightclient
        src/BlsVoterSet.cpp
        src/Checkpoint.cpp
        src/CommitteeInfo.cpp
        src/CommitteeSet.cpp
        src/ConfirmPoint.cpp
        src/EpochEndPoint.cpp
        ${HEADERS} )

target_link_libraries( ultrainio_lightclient ultrainio_core )
target_include_directories( ultrainio_lightclient PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
