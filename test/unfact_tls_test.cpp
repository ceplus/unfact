
#include <unfact/extras/thread_local.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>

namespace uf  = unfact;
namespace uex = unfact::extras;

void test_tls_hello()
{
  uex::default_thread_local_t<10>::type tls;
  uf::byte_t x;
  tls.set(&x);
  UF_TEST_EQUAL(&x, tls.get());
}

void test_tls()
{
  test_tls_hello();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
