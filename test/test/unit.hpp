
#ifndef UNFACT_TEST_UNIT_HPP
#define UNFACT_TEST_UNIT_HPP

/*
 *
 * a minimum unit-testing framework: depend on 
 *
 */

#include <stddef.h>

/*
 * @return always true. we return some value just for short code on user-side.
 *         see UF_TEST() for example.
 */
bool unfact_test_failed(const char* file, size_t line, const char* cond); 

#define UF_TEST(cond)       ((cond) || unfact_test_failed(__FILE__, __LINE__, "<" #cond "> is not satisfied!"))
#define UF_TEST_EQUAL(a, b) (((a) == (b)) ||								\
                             unfact_test_failed(__FILE__, __LINE__, "<" #a "> is not <" #b ">!"))
#define UF_TEST_NOTEQ(a, b) (((a) != (b)) ||								\
                             unfact_test_failed(__FILE__, __LINE__, "<" #a "> is not <" #b ">!"))

#endif//UNFACT_TEST_UNIT_HPP
