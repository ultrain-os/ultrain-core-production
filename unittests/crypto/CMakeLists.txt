#bls
add_executable( bls_test
        BlsTest.cpp)

target_link_libraries( bls_test ultrainio_crypto )

add_test(NAME bls_test COMMAND bls_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

#private/public key
add_executable( pripubkey_test
        PriPubKeyTest.cpp)

target_link_libraries( pripubkey_test ultrainio_crypto )

add_test(NAME pripubkey_test COMMAND pripubkey_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})