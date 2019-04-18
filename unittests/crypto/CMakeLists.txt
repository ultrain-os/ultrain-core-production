FIND_PACKAGE(Boost 1.67 REQUIRED COMPONENTS
        chrono
        unit_test_framework
        iostreams)

# bls
add_executable( bls_test
        BlsTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( bls_test ultrainio_crypto )

add_test(NAME bls_test COMMAND bls_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

#private/public key
add_executable( pripubkey_test
        PriPubKeyTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( pripubkey_test ultrainio_crypto )

add_test(NAME pripubkey_test COMMAND pripubkey_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

#Signer/Validator
add_executable( signervalidator_test
        SignerValidatorTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( signervalidator_test ultrainio_crypto ultrainio_core)

add_test(NAME signervalidator_test COMMAND signervalidator_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

#Random
add_executable( random_unittest
        RandomTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( random_unittest ultrainio_crypto)

add_test(NAME random_unittest COMMAND random_unittest WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
