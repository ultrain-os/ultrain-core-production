add_executable( bls_test
        BlsTest.cpp)

target_link_libraries( bls_test ultrainio_crypto )

#Ed25519 performance test
add_executable( ed25519_performance_test
        Ed25519PerformanceTest.cpp)

target_link_libraries( ed25519_performance_test ultrainio_crypto fc)
