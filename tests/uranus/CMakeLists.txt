add_executable( votersystem_test
        VoterSystemTest.cpp)

target_link_libraries( votersystem_test ultrainio_crypto ultrainio_uranus )

add_executable( signervalidator_test
        SignerValidatorTest.cpp)

target_link_libraries( signervalidator_test ultrainio_crypto ultrainio_uranus )
