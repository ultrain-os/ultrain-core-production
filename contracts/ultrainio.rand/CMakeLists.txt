
file(GLOB ABI_FILES "*.abi")
configure_file("${ABI_FILES}" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

file(GLOB WASM_FILES "ultrainio.rand.wasm.m")
configure_file("${WASM_FILES}" "${CMAKE_CURRENT_BINARY_DIR}/ultrainio.rand.wasm" COPYONLY)

file(GLOB WAST_FILES "ultrainio.rand.wast.t")
configure_file("${WAST_FILES}" "${CMAKE_CURRENT_BINARY_DIR}/ultrainio.rand.wast" COPYONLY)
