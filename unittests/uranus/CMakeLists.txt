add_executable( MessageManagerTest
        MessageManagerTest.cpp)

target_link_libraries( MessageManagerTest gtest_main ultrainio_crypto ultrainio_uranus )
