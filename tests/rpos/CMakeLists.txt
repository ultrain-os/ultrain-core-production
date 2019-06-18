#FisherYates performance test
add_executable( fisheryates_performance_test
        FisherYatesPerformanceTest.cpp )

target_link_libraries( fisheryates_performance_test ultrainio_rpos )
