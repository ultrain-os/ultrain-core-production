#include <ultrainiolib/real.hpp>
#include <ultrainiolib/ultrainio.hpp>

#include "test_api.hpp"

void test_real::create_instances() {
    ultrainio::real lhs1(5);
    ultrainio_assert(lhs1.value() == 5, "real instance value is wrong");
}

void test_real::test_division() {
    ultrainio::real lhs1(5);
    ultrainio::real rhs1(10);
    ultrainio::real result1 = lhs1 / rhs1;

    uint64_t a = double_div(i64_to_double(5), i64_to_double(10));
    ultrainio_assert(a == result1.value(), "real division result is wrong");
}

void test_real::test_division_by_0() {
    ultrainio::real lhs1(5);
    ultrainio::real rhs1(0);
    ultrainio::real result1 = lhs1 / rhs1;
    // in order to get rid of unused parameter warning
    result1 = 0;

    ultrainio_assert(false, "should've thrown an error");
}

void test_real::test_multiplication() {
    ultrainio::real lhs1(5);
    ultrainio::real rhs1(10);
    ultrainio::real result1 = lhs1 * rhs1;
    uint64_t res = double_mult( 5, 10 );
    ultrainio_assert(res == result1.value(), "real multiplication result is wrong");
}

void test_real::test_addition()
{
    ultrainio::real lhs1(5);
    ultrainio::real rhs1(10);
    ultrainio::real result1 = lhs1 / rhs1;
    uint64_t a = double_div(i64_to_double(5), i64_to_double(10));

    ultrainio::real lhs2(5);
    ultrainio::real rhs2(2);
    ultrainio::real result2 = lhs2 / rhs2;
    uint64_t b = double_div(i64_to_double(5), i64_to_double(2));


    ultrainio::real sum = result1+result2;
    uint64_t c = double_add( a, b );
    ultrainio_assert(sum.value() == c, "real addition operation result is wrong");
}


