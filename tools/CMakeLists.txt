add_executable( keypair_generator
        KeyPairGenerator.cpp)
target_link_libraries( keypair_generator ultrainio_crypto )

add_executable( ws2json ws2json.cpp)
target_include_directories( ws2json PRIVATE ${Boost_INCLUDE_DIR} )
target_link_libraries( ws2json PRIVATE fc ultrainio_chain chainbase)

add_executable( sk2bls_keypair
        Sk2BlsKeypair.cpp)
target_link_libraries( sk2bls_keypair ultrainio_base ultrainio_crypto )
