add_executable( keypair_generator
        KeyPairGenerator.cpp)

target_link_libraries( keypair_generator ultrainio_crypto )
