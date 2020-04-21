# ed25519

file(GLOB HEADERS "include/crypto/ed25519/*.h")
add_library( ultrainio_ed25519
        src/ed25519/Digest.cpp
        src/ed25519/Ed25519.cpp
        src/ed25519/PrivateKey.cpp
        src/ed25519/PublicKey.cpp
        src/ed25519/Signature.cpp
        ${HEADERS} )

include_directories( "${PROJECT_SOURCE_DIR}/vendor/boringssl/include" )

target_link_libraries( ultrainio_ed25519 ultrainio_pbc ultrainio_base fiat fc)
target_include_directories( ultrainio_ed25519 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/crypto" )

# gm

file(GLOB HEADERS "include/crypto/gm/sm2/*.h")

add_library( ultrainio_gm
        src/gm/sm2/ec.h
        src/gm/sm2/numbers.h
        src/gm/sm2/sm2.h
        src/gm/sm2/sm2_err.c
        src/gm/sm2/sm2_sign.c
        src/gm/sm2/sm2_err.c
        src/gm/sm2/PrivateKey.cpp
        src/gm/sm2/PublicKey.cpp
        src/gm/sm2/Signature.cpp
        src/gm/sm3/Digest.cpp
        ${HEADERS} )

target_link_libraries( ultrainio_gm ultrainio_base fc)

target_include_directories( ultrainio_gm PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/crypto" )

# ultrainio_crypto
file(GLOB HEADERS "include/crypto/*.h")
add_library( ultrainio_crypto
        src/Bls.cpp
        src/Random.cpp
        ${HEADERS} )

#include_directories( "${PROJECT_SOURCE_DIR}/vendor/boringssl/include" )

target_link_libraries( ultrainio_crypto ultrainio_gm ultrainio_ed25519)
target_include_directories( ultrainio_crypto PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/" )
