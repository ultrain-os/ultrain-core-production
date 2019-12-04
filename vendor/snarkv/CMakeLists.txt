project (snarkv)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -fPIC -Wno-unused-variable -DCURVE_ALT_BN128 -DNO_PT_COMPRESSION -DSERVER_SIDE")
file(GLOB HEADERS "include/*.h")
include_directories(
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/snark/libsnark
  ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/fc/include
  ${Boost_INCLUDE_DIR}
)

add_library(snarkv src/groth16.cpp src/incremental_merkle_tree.cpp src/uint256.cpp
	           src/util_str_encodings.cpp src/util.cpp src/signature.cpp
                   src/address.cpp src/note_encryption.cpp src/hash.cpp src/prf.cpp
		   src/sha256.cpp src/sha512.cpp src/stransaction_builder.cpp
                   src/stransaction.cpp src/note.cpp ${HEADERS})

target_link_libraries(snarkv  fc ${CMAKE_CURRENT_SOURCE_DIR}/snark/libsnark.a ${CMAKE_CURRENT_SOURCE_DIR}/snark/librustzcash.a ${CMAKE_CURRENT_SOURCE_DIR}/snark/libsodium.a gmpxx gmp)

