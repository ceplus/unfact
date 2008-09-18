
#include <unfact/arena.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>
#include <vector>

namespace uf = unfact;
typedef uf::basic_arena_t<uf::default_concurrent_t> arena_type;

static void
test_arena_hello()
{
  tracing_allocator_t allocator;
  arena_type arena(&allocator, 10, 100);
  uf::byte_t* ptr = arena.allocate();
  UF_TEST(ptr);
  arena.deallocate(ptr);
}

static void
test_arena_item_align()
{
  tracing_allocator_t allocator;
  arena_type a0(&allocator, 10, 100, 2);
  UF_TEST_EQUAL(10, a0.item_size());
  arena_type a1(&allocator, 10, 100, 4);
  UF_TEST_EQUAL(12, a1.item_size());
}

static void
test_arena_allocate_from_page()
{
  tracing_allocator_t allocator;
  std::vector<uf::byte_t*> items;

  arena_type arena(&allocator, 10, 30, 2);
  UF_TEST_EQUAL(0, arena.npages());
  UF_TEST_EQUAL(0, arena.size());
  UF_TEST_EQUAL(0, arena.nslots());
  UF_TEST_EQUAL(0, arena.npacked());
  
  items.push_back(arena.allocate());
  UF_TEST_EQUAL(1, arena.npages());
  UF_TEST_EQUAL(1, arena.size());
  UF_TEST_EQUAL(0, arena.nslots());
  UF_TEST_EQUAL(2, arena.npacked());

  items.push_back(arena.allocate());
  UF_TEST_EQUAL(1, arena.npages());
  UF_TEST_EQUAL(2, arena.size());
  UF_TEST_EQUAL(0, arena.nslots());
  UF_TEST_EQUAL(3, arena.npacked());

  items.push_back(arena.allocate());
  UF_TEST_EQUAL(2, arena.npages());
  UF_TEST_EQUAL(3, arena.size());
  UF_TEST_EQUAL(0, arena.nslots());
  UF_TEST_EQUAL(2, arena.npacked());

  arena.deallocate(items[0]);
  UF_TEST_EQUAL(2, arena.npages());
  UF_TEST_EQUAL(2, arena.size());
  UF_TEST_EQUAL(1, arena.nslots());
  UF_TEST_EQUAL(2, arena.npacked());

  arena.deallocate(items[1]);
  UF_TEST_EQUAL(2, arena.npages());
  UF_TEST_EQUAL(1, arena.size());
  UF_TEST_EQUAL(2, arena.nslots());
  UF_TEST_EQUAL(2, arena.npacked());

  arena.deallocate(items[2]);
  UF_TEST_EQUAL(2, arena.npages());
  UF_TEST_EQUAL(0, arena.size());
  UF_TEST_EQUAL(3, arena.nslots());
  UF_TEST_EQUAL(2, arena.npacked());

}

static void
test_arena_allocate_from_slot()
{
  tracing_allocator_t allocator;
  std::vector<uf::byte_t*> items;

  arena_type arena(&allocator, 10, 30, 2);

  uf::byte_t* p0 = arena.allocate();
  uf::byte_t* p1 = arena.allocate();
  uf::byte_t* p2 = arena.allocate();

  arena.deallocate(p0);
  uf::byte_t* p3 = arena.allocate();
  UF_TEST_EQUAL(2, arena.npages());
  UF_TEST_EQUAL(3, arena.size());
  UF_TEST_EQUAL(0, arena.nslots());
  UF_TEST_EQUAL(2, arena.npacked());
  
  arena.deallocate(p1);
  arena.deallocate(p2);
  uf::byte_t* p4 = arena.allocate();
  UF_TEST_EQUAL(2, arena.npages());
  UF_TEST_EQUAL(2, arena.size());
  UF_TEST_EQUAL(1, arena.nslots());
  UF_TEST_EQUAL(2, arena.npacked());

  arena.deallocate(p3);
  arena.deallocate(p4);

}

static void
test_arena_allocate_fail_first()
{
  fail_allocator_t al(0);
  arena_type ar(&al, 10, 30, 2);
  uf::byte_t* p = ar.allocate();
  UF_TEST(!p);
}

static void
test_arena_allocate_fail_second()
{
  fail_allocator_t al(1);
  arena_type ar(&al, 10, 30, 2);
  uf::byte_t* p0 = ar.allocate();
  UF_TEST( p0);
  uf::byte_t* p1 = ar.allocate();
  UF_TEST( p1);
  uf::byte_t* p2 = ar.allocate();
  UF_TEST(!p2);
  //printf("item:%d, page:%d, slot:%d\n", ar.size(), ar.npages(), ar.nslots());
  ar.deallocate(p1);
  ar.deallocate(p0);
}

static void
test_arena_swap()
{
  tracing_allocator_t al0;
  tracing_allocator_t al1;

  arena_type ar0(&al0, 16, 200);
  arena_type ar1(&al1, 32, 300);

  uf::byte_t* p0  = ar0.allocate();
  uf::byte_t* p1a = ar1.allocate();
  uf::byte_t* p1b = ar1.allocate();

  ar0.exchange(ar1);

  UF_TEST_EQUAL(ar0.item_size(),  32);
  UF_TEST_EQUAL(ar0.page_size(), 300);
  UF_TEST_EQUAL(ar0.size(),      2);
  UF_TEST_EQUAL(ar0.allocator(), &al1);

  UF_TEST_EQUAL(ar1.item_size(),  16);
  UF_TEST_EQUAL(ar1.page_size(), 200);
  UF_TEST_EQUAL(ar1.size(),      1);
  UF_TEST_EQUAL(ar1.allocator(), &al0);

  ar0.exchange(ar0); // self exchange: nothing should happens
  UF_TEST_EQUAL(ar0.item_size(),  32);
  UF_TEST_EQUAL(ar0.page_size(), 300);
  UF_TEST_EQUAL(ar0.size(),      2);
  UF_TEST_EQUAL(ar0.allocator(), &al1);

  ar0.deallocate(p1a);
  ar0.deallocate(p1b);
  ar1.deallocate(p0);


}

void test_arena()
{
  test_arena_hello();
  test_arena_item_align();
  test_arena_allocate_from_page();
  test_arena_allocate_from_slot();
  test_arena_allocate_fail_first();
  test_arena_allocate_fail_second();
  test_arena_swap();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
