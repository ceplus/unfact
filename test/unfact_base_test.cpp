
#include <unfact/arena.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>
#include <vector>

namespace uf = unfact;

static void
test_roundup_to_p2()
{
  UF_TEST_EQUAL( 0, uf::roundup_to_p2( 0, 8));
  UF_TEST_EQUAL( 8, uf::roundup_to_p2( 1, 8));
  UF_TEST_EQUAL( 8, uf::roundup_to_p2( 7, 8));
  UF_TEST_EQUAL( 8, uf::roundup_to_p2( 8, 8));
  UF_TEST_EQUAL(16, uf::roundup_to_p2(10, 8));
  UF_TEST_EQUAL(16, uf::roundup_to_p2(16, 8));
}

void test_base()
{
  test_roundup_to_p2();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
