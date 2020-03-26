#GM test
add_executable( gm_test
        GmTest.cpp)

target_link_libraries( gm_test ultrainio_gm )

add_executable( gmprofile_test
        GmProfileTest.cpp)

target_link_libraries( gmprofile_test ultrainio_gm )
