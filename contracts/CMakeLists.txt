# will be implictly used for any compilation unit if not overrided by SYSTEM_INCLUDE_FOLDERS parameter
# these directories go as -isystem <dir> to avoid warnings from code of third-party libraries
set(DEFAULT_SYSTEM_INCLUDE_FOLDERS ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include ${Boost_INCLUDE_DIR})

set(STANDARD_INCLUDE_FOLDERS ${CMAKE_SOURCE_DIR}/contracts ${CMAKE_BINARY_DIR}/contracts ${CMAKE_SOURCE_DIR}/vendor/magic_get/include)

add_subdirectory(ultrainiolib)
add_subdirectory(musl)
add_subdirectory(libc++)
add_subdirectory(simple.token)
add_subdirectory(ultrainio.token)
add_subdirectory(ultrainio.msig)
add_subdirectory(ultrainio.rand)
add_subdirectory(ultrainio.sudo)
add_subdirectory(ultrainio.bank)
add_subdirectory(multi_index_test)
add_subdirectory(ultrainio.system)
add_subdirectory(identity)
add_subdirectory(stltest)
add_subdirectory(exchange)
add_subdirectory(test.inline)
add_subdirectory(economic)
add_subdirectory(merkle_proof)

#add_subdirectory(bancor)
add_subdirectory(hello)
add_subdirectory(asserter)
add_subdirectory(infinite)
add_subdirectory(proxy)
add_subdirectory(test_api)
add_subdirectory(test_api_mem)
add_subdirectory(test_api_db)
add_subdirectory(test_api_multi_index)
add_subdirectory(test_ram_limit)
#add_subdirectory(social)
add_subdirectory(noop)
add_subdirectory(dice)
add_subdirectory(tic_tac_toe)
add_subdirectory(payloadless)


file(GLOB SKELETONS RELATIVE ${CMAKE_SOURCE_DIR}/contracts "skeleton/*")

# Documented multiple output support is broken, so properly setting up the multiple
# dependencies in the custom target is not possible.  (CMake 3.5)
add_custom_command(OUTPUT share/ultrainio/skeleton/skeleton.cpp
                   COMMAND ${CMAKE_COMMAND} -E make_directory ../share/ultrainio/skeleton
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/skeleton ../share/ultrainio/skeleton
                   DEPENDS ${SKELETONS}
                   COMMENT Copying skeleton contract...
                   VERBATIM)
add_custom_target(copy_skeleton_contract ALL DEPENDS share/ultrainio/skeleton/skeleton.cpp)
