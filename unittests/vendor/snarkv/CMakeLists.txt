add_executable( snarkv_test ZkpInterfaceTest.cpp)

include_directories(
   PUBLIC 
       ${CMAKE_CURRENT_SOURCE_DIR}/../../../vendor/snarkv/include
       ${CMAKE_CURRENT_SOURCE_DIR}/../../../vendor/snarkv/snark
       ${CMAKE_CURRENT_SOURCE_DIR}/../../../vendor/libraries/fc/include
       ${Boost_INCLUDE_DIR} )


target_link_libraries( snarkv_test snarkv dl ${Boost_LIBRARIES} )
#target_link_libraries( snarkv_test snarkv dl ${Boost_LIBRARIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../../build/vendor/snarkv/libsnarkv.a)
#target_link_libraries( snarkv_test snarkv ${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/snarkv/snark/libsnark.a ${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/snarkv/snark/librustzcash.a ${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/snarkv/snark/libsodium.a)
add_test(NAME snarkv_test COMMAND snarkv_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
