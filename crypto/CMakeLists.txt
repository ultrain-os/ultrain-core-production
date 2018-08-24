file(GLOB HEADERS "include/crypto/*.h")
add_library( ultrainio_crypto
        src/Bls.cpp
        src/Digest.cpp
        src/Ed25519.cpp
        src/PrivateKey.cpp
        src/PublicKey.cpp
        src/Signature.cpp
        src/Vrf.cpp
        ${HEADERS} )

include_directories( "${PROJECT_SOURCE_DIR}/vendor/boringssl/include" )

target_link_libraries( ultrainio_crypto ultrainio_pbc ultrainio_base fiat crypto fc)
target_include_directories( ultrainio_crypto PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
