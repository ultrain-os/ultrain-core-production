add_executable( bls_performance_test
        BlsPerformanceTest.cpp )

target_link_libraries( bls_performance_test ultrainio_crypto )

#sha256 performance test
add_executable( sha256_performance_test
        SHA256PerformanceTest.cpp )

target_link_libraries( sha256_performance_test fc )

#Random test
add_executable( random_test
        RandomTest.cpp )

target_link_libraries( random_test ultrainio_crypto )

add_subdirectory( gm )
