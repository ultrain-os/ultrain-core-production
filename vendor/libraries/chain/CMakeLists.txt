configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/ultrainio/chain/core_symbol.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/ultrainio/chain/core_symbol.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/genesis_state_root_key.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/genesis_state_root_key.cpp)

file(GLOB HEADERS "include/ultrainio/chain/*.hpp"
                  "include/ultrainio/chain/webassembly/*.hpp"
                  "${CMAKE_CURRENT_BINARY_DIR}/include/ultrainio/chain/core_symbol.hpp" )

## SORT .cpp by most likely to change / break compile
add_library( ultrainio_chain
             merkle.cpp
             name.cpp
	     name_ex.cpp
             transaction.cpp
             block_header.cpp
             block_header_state.cpp
             block_state.cpp
             fork_database.cpp
             controller.cpp
             authorization_manager.cpp
             resource_limits.cpp
             block_log.cpp
             transaction_context.cpp
             ultrainio_contract.cpp
             ultrainio_contract_abi.cpp
             chain_config.cpp
             chain_id_type.cpp
             genesis_state.cpp
             ${CMAKE_CURRENT_BINARY_DIR}/genesis_state_root_key.cpp

#             chain_config.cpp
#             block_trace.cpp
              wast_to_wasm.cpp
              wasm_interface.cpp
              wasm_ultrainio_validation.cpp
              wasm_ultrainio_injection.cpp
              apply_context.cpp
              abi_serializer.cpp
              asset.cpp

             webassembly/wavm.cpp
             webassembly/binaryen.cpp

             worldstate
             worldstate_file_manager

	     webassembly/wabt.cpp
             config.cpp
	     block_timestamp.cpp
#             get_config.cpp
#             global_property_object.cpp
#
#             contracts/chain_initializer.cpp


#             transaction_metadata.cpp
             ${HEADERS}
             )

add_dependencies(ultrainio_chain  ultrainio.token)
target_link_libraries( ultrainio_chain ultrain_utilities ultrainio_crypto fc chainbase appbase chain_plugin Logging IR WAST WASM Runtime
      wasm asmjs passes cfg ast emscripten-optimizer support softfloat builtins wabt
                     )
target_include_directories( ultrainio_chain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../../../vendor/binaryen/src"
                                   "${CMAKE_BINARY_DIR}/contracts"
		                           "${CMAKE_SOURCE_DIR}/vendor/libraries/wabt"
                                   "${CMAKE_BINARY_DIR}/vendor/libraries/wabt"
                            )

install( TARGETS ultrainio_chain
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
)
install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ultrainio/chain/
      DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/ultrainio/chain
      FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" PATTERN "webassembly" EXCLUDE
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/ultrainio/chain/core_symbol.hpp DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/ultrainio/chain)
#if(MSVC)
#  set_source_files_properties( db_init.cpp db_block.cpp database.cpp block_log.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
#endif(MSVC)
