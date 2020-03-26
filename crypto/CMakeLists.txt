file(GLOB HEADERS "include/crypto/*.h")
add_library( ultrainio_crypto
        src/Bls.cpp
        src/Digest.cpp
        src/Ed25519.cpp
        src/PrivateKey.cpp
        src/PublicKey.cpp
        src/Random.cpp
        src/Signature.cpp
        ${HEADERS} )

include_directories( "${PROJECT_SOURCE_DIR}/vendor/boringssl/include" )

target_link_libraries( ultrainio_crypto ultrainio_pbc ultrainio_base fiat fc)
target_include_directories( ultrainio_crypto PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

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
