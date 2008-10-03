
#include <unfact/concurrent.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>
#include <vector>
#include <algorithm>
#include <string>

namespace uf = unfact;

void test_concurrent_spin_lock_hello()
{
  uf::default_concurrent_t::spin_lock_type l;
  l.acquire();
  l.release();
  l.acquire();
  l.release();
}

void test_concurrent_rw_lock_hello()
{
  uf::default_concurrent_t::rw_lock_type rw;
  rw.read_acquire();
  rw.read_acquire();
  rw.read_release();
  rw.read_release();
  rw.read_acquire();
  rw.read_release();

  rw.write_acquire();
  rw.write_release();
  rw.write_acquire();
  rw.write_release();
}

void test_concurrent()
{
  test_concurrent_spin_lock_hello();
  test_concurrent_rw_lock_hello();
}


/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
