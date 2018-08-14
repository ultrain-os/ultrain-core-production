file(GLOB HEADERS "include/crypto/*.h")
add_library( ultrainio_crypto
        src/Bls.cpp
        src/Vrf.cpp
        ${HEADERS} )

include_directories( "${PROJECT_SOURCE_DIR}/vendor/boringssl/include" )

target_link_libraries( ultrainio_crypto ultrainio_pbc fiat crypto )
target_include_directories( ultrainio_crypto PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
