file(GLOB HEADERS "include/*.h" )

add_library( ultrainio_pbc
        arith/field.c
        arith/z.c
        arith/naivefp.c
        arith/fastfp.c
        arith/fp.c
        arith/fasterfp.c
        arith/montfp.c
        arith/ternary_extension_field.c
        arith/multiz.c
        arith/dlog.c
        arith/fieldquadratic.c
        arith/poly.c
        arith/random.c
        arith/init_random.c
        misc/darray.c
        misc/symtab.c
        misc/get_time.c
        misc/utils.c
        misc/memory.c
        misc/extend_printf.c
        ecc/mpc.c
        ecc/mnt.c
        ecc/hilbert.c
        ecc/curve.c
        ecc/pairing.c
        ecc/singular.c
        ecc/eta_T_3.c
        ecc/param.c
        ecc/a_param.c
        ecc/d_param.c
        ecc/e_param.c
        ecc/f_param.c
        ecc/g_param.c
        ${HEADERS} )

find_package(GMP REQUIRED)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
target_link_libraries( ultrainio_pbc ${GMP_LIBRARIES} m )
target_include_directories( ultrainio_pbc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" ${GMP_INCLUDE_DIR} )
