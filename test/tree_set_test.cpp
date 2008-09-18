
#include <unfact/tree_set.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

namespace uf = unfact;
typedef uf::tree_set_t<int, uf::less_t<int>, uf::default_concurrent_t> int_tree_set;

static void
test_tree_set_hello()
{
  tracing_allocator_t alloc;
  int_tree_set s(&alloc, 1024);
}

static void
test_tree_set_instantiate_iterators()
{
  tracing_allocator_t alloc;
  int_tree_set s(&alloc, 1024);
  int_tree_set::iterator b = s.begin();
  int_tree_set::iterator e = s.end();
  int_tree_set::iterator i;
  UF_TEST(  b == e);
  UF_TEST(!(b != e));

  const int_tree_set cs(&alloc, 1024);
  int_tree_set::const_iterator cb = cs.begin();
  int_tree_set::const_iterator ce = cs.end();
  int_tree_set::const_iterator ci;
  UF_TEST(  cb == ce);
  UF_TEST(!(cb != ce));

  int_tree_set::const_iterator cf = cs.find(1);
  UF_TEST(  ce == cf);
}

static void
test_tree_set_insert_remove_size()
{
  tracing_allocator_t alloc;
  int_tree_set ts(&alloc, 1024);
  
  int_tree_set::iterator i1 = ts.insert(1);
  UF_TEST_EQUAL(1, *i1);
  int_tree_set::iterator i2 = ts.insert(2);
  UF_TEST_EQUAL(2, *i2);
  int_tree_set::iterator i3 = ts.insert(3);
  UF_TEST_EQUAL(3, *i3);
  UF_TEST_EQUAL(3, ts.count());
  ts.remove(i1);
  ts.remove(i2);
  ts.remove(i3);
  UF_TEST_EQUAL(0, ts.count());
}

static void
test_tree_set_empty()
{
  tracing_allocator_t alloc;
  int_tree_set ts(&alloc, 1024);

  UF_TEST( ts.empty());
  int_tree_set::iterator i = ts.insert(1);
  UF_TEST(!ts.empty());
  ts.remove(i);
  UF_TEST( ts.empty());
}

static void
test_tree_set_clear()
{
  tracing_allocator_t alloc;
  int_tree_set ts(&alloc, 1024);

  ts.insert(1);
  ts.insert(2);
  ts.insert(3);
  UF_TEST_EQUAL(3, ts.count());

  ts.clear();
  UF_TEST(ts.empty());
  UF_TEST_EQUAL(0, ts.count());
}

template<class Iterator>
void
do_test_tree_set_iterator_move_postfix()
{
  tracing_allocator_t alloc;
  int_tree_set ts(&alloc, 1024);

  ts.insert(1);
  ts.insert(2);
  ts.insert(3);

  Iterator l;
  Iterator i = ts.find(1);
  UF_TEST_EQUAL(*i, 1);
  UF_TEST(!i.atend());

  l = i++;
  UF_TEST_EQUAL(*l, 1);
  UF_TEST_EQUAL(*i, 2);

  l = i++;
  UF_TEST_EQUAL(*l, 2);
  UF_TEST_EQUAL(*i, 3);

  l = i++;
  UF_TEST_EQUAL(*l, 3);
  UF_TEST(i.atend());

  i = l;
  
  l = i--;
  UF_TEST_EQUAL(*l, 3);
  UF_TEST_EQUAL(*i, 2);

  l = i--;
  UF_TEST_EQUAL(*l, 2);
  UF_TEST_EQUAL(*i, 1);

  l = i--;
  UF_TEST_EQUAL(*l, 1);
  UF_TEST(i.atend());
  
}

void
test_tree_set_iterator_move_postfix()
{
  do_test_tree_set_iterator_move_postfix<int_tree_set::iterator>();
  do_test_tree_set_iterator_move_postfix<int_tree_set::const_iterator>();
}

template<class Iterator>
void
do_test_tree_set_iterator_move_prefix()
{
  tracing_allocator_t alloc;
  int_tree_set ts(&alloc, 1024);

  ts.insert(1);
  ts.insert(2);
  ts.insert(3);

  Iterator l;
  Iterator i = ts.find(1);
  UF_TEST_EQUAL(*i, 1);
  UF_TEST(!i.atend());

  l = ++i;
  UF_TEST_EQUAL(*l, 2);
  UF_TEST_EQUAL(*i, 2);

  l = ++i;
  UF_TEST_EQUAL(*l, 3);
  UF_TEST_EQUAL(*i, 3);
  
  l = --i;
  UF_TEST_EQUAL(*l, 2);
  UF_TEST_EQUAL(*i, 2);

  l = --i;
  UF_TEST_EQUAL(*l, 1);
  UF_TEST_EQUAL(*i, 1);

  l = --i;
  UF_TEST(l.atend());
  UF_TEST(i.atend());
}

void
test_tree_set_iterator_move_prefix()
{
  do_test_tree_set_iterator_move_prefix<int_tree_set::iterator>();
  do_test_tree_set_iterator_move_prefix<int_tree_set::const_iterator>();
}

void
test_tree_set_copy()
{
  tracing_allocator_t alloc;
  int_tree_set ts(&alloc, 1024);

  ts.insert(1);
  ts.insert(2);
  ts.insert(3);
  UF_TEST_EQUAL(ts.count(), 3);

  int_tree_set copy = ts;
  UF_TEST_EQUAL(copy.count(), 3);
  UF_TEST(copy.contains(1));
  UF_TEST(copy.contains(2));
  UF_TEST(copy.contains(3));
}

void
test_tree_set_assign()
{
  tracing_allocator_t a1;
  tracing_allocator_t a2;
  int_tree_set s1(&a1, 1024);
  int_tree_set s2(&a2, 1024);

  s1.insert(1);
  s1.insert(2);
  s1.insert(3);

  s2.insert(4);
  s2.insert(5);
  UF_TEST_EQUAL(s2.count(), 2);
  
  s2 = s1;
  UF_TEST_EQUAL(s2.allocator(), &a1);
  UF_TEST_EQUAL(s2.count(), 3);
  UF_TEST(s2.contains(1));
  UF_TEST(s2.contains(2));
  UF_TEST(s2.contains(3));

  UF_TEST_EQUAL(s1.count(), 3);
}

void
test_tree_set_ensure()
{
  tracing_allocator_t alloc;
  int_tree_set ts(&alloc, 1024);

  int_tree_set::const_iterator i0 = ts.ensure(100);
  int_tree_set::const_iterator i1 = ts.ensure(100);
  int_tree_set::const_iterator i2 = ts.ensure(200);

  UF_TEST(i0 != ts.end());
  UF_TEST(i0 == i1);
  UF_TEST(i0 != i2);
}

void
test_tree_set_exchange()
{
  tracing_allocator_t alloc;
  int_tree_set s0(&alloc, 1024);
  int_tree_set s1(&alloc, 1024);

  s0.insert(1);
  s0.insert(2);
  s0.insert(3);
  s1.insert(4);
  s1.insert(5);
  s0.exchange(s1);

  UF_TEST(!s0.contains(1));
  UF_TEST(!s0.contains(2));
  UF_TEST(!s0.contains(3));
  UF_TEST( s0.contains(4));
  UF_TEST( s0.contains(5));
  UF_TEST( s1.contains(1));
  UF_TEST( s1.contains(2));
  UF_TEST( s1.contains(3));
  UF_TEST(!s1.contains(4));
  UF_TEST(!s1.contains(5));
}

void test_tree_set()
{
  test_tree_set_hello();
  test_tree_set_instantiate_iterators();
  test_tree_set_insert_remove_size();
  test_tree_set_empty();
  test_tree_set_clear();
  test_tree_set_iterator_move_postfix();
  test_tree_set_iterator_move_prefix();
  test_tree_set_copy();
  test_tree_set_assign();
  test_tree_set_ensure();
  test_tree_set_exchange();
}


/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
