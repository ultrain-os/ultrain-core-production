### CommitteeSet
add_executable( committeeset_test_suite
        CommitteeSetTest.cpp)

include_directories ( ${Boost_INCLUDE_DIR} )

target_link_libraries( committeeset_test_suite ultrainio_lightclient )

add_test(NAME committeeset_test_suite COMMAND committeeset_test_suite WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
