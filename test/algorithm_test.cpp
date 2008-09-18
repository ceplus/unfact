
#include <unfact/algorithm.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>
#include <iostream>

void test_swap_hello()
{
  int x = 10;
  int y = 20;
  unfact::exchange(x, y);
  UF_TEST_EQUAL(x, 20);
  UF_TEST_EQUAL(y, 10);
}


void test_algorithm()
{
  test_swap_hello();
}


/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
