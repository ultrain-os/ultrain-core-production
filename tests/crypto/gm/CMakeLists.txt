#GM profile test
add_executable( gmprofile_test
        GmProfileTest.cpp)

target_link_libraries( gmprofile_test ultrainio_gm ultrainio_ed25519)
