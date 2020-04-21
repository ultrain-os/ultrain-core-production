add_executable( bls_performance_test
        BlsPerformanceTest.cpp )

target_link_libraries( bls_performance_test ultrainio_crypto )

#Ed25519 performance test
add_executable( ed25519_performance_test
        Ed25519PerformanceTest.cpp )

target_link_libraries( ed25519_performance_test ultrainio_ed25519 fc )

#Ed25519 performance test
add_executable( sha256_performance_test
        SHA256PerformanceTest.cpp )

target_link_libraries( sha256_performance_test fc )

#Random test
add_executable( random_test
        RandomTest.cpp )

target_link_libraries( random_test ultrainio_crypto )

add_subdirectory( gm )
