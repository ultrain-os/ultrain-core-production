add_executable( bls_test
        BlsTest.cpp)

target_link_libraries( bls_test ultrainio_crypto )

add_executable( vrf_test
        VrfTest.cpp)

target_link_libraries( vrf_test ultrainio_crypto )