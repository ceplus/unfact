
#include <unfact/set_tree.hpp>
#include <test/memory_support.hpp>
#include <test/instance_counted.hpp>
#include <test/unit.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

namespace uf = unfact;
typedef uf::set_tree_t<int, uf::less_t<int>, uf::default_concurrent_t> int_set_tree;

static void
test_set_tree_hello()
{
  tracing_allocator_t alloc;
  int_set_tree s(10, &alloc, 1024);
  UF_TEST_EQUAL(*s.child_begin(), 10);
}

namespace
{
  class test_set_tree_insert_remove_tag_t {};
}

static void
test_set_tree_insert_remove()
{
  typedef instance_counted_t<test_set_tree_insert_remove_tag_t> my_instance_counted_t;
  typedef uf::set_tree_t<my_instance_counted_t> ic_set_tree;

  tracing_allocator_t alloc;
  ic_set_tree s(0, &alloc, 1024);
  ic_set_tree::child_iterator_t i1  = s.insert(s.child_begin(), 10);
  ic_set_tree::child_iterator_t i2  = s.insert(s.child_begin(), 20);
  ic_set_tree::child_iterator_t i3  = s.insert(s.child_begin(), 30);
  ic_set_tree::child_iterator_t i22 = s.insert(i2, 22);
  ic_set_tree::child_iterator_t i21 = s.insert(i2, 21);
  ic_set_tree::child_iterator_t i23 = s.insert(i2, 23);
  UF_TEST_EQUAL(my_instance_counted_t::instances(), 6 + 1); // +1 for root node

  UF_TEST_EQUAL(i1.node()->children().root(), 0);
  UF_TEST_EQUAL(i1.node()->parent(), s.root());
  UF_TEST_EQUAL(i2.node()->children().root(), i22.node());
  UF_TEST_EQUAL(i2.node()->parent(), s.root());
  UF_TEST_EQUAL(i3.node()->children().root(), 0);
  UF_TEST_EQUAL(i3.node()->parent(), s.root());
  UF_TEST_EQUAL(i22.node()->parent(), i2.node());
  UF_TEST_EQUAL(i22.node()->children().root(), 0);

  s.remove(i1); 
  UF_TEST_EQUAL(my_instance_counted_t::instances(), 5 + 1);
  s.remove(i2); // should remove recursively
  UF_TEST_EQUAL(my_instance_counted_t::instances(), 1 + 1);
  s.remove(i3); 
  UF_TEST_EQUAL(my_instance_counted_t::instances(), 0 + 1);
}

namespace
{
  class test_set_tree_clear_tag_t {};
}

static void
test_set_tree_clear()
{
  typedef instance_counted_t<test_set_tree_clear_tag_t> my_instance_counted_t;
  typedef uf::set_tree_t< my_instance_counted_t > ic_set_tree;

  tracing_allocator_t alloc;
  ic_set_tree s(0, &alloc, 1024);
  ic_set_tree::child_iterator_t i1  = s.insert(s.child_begin(), 10);
  ic_set_tree::child_iterator_t i2  = s.insert(s.child_begin(), 20);
  ic_set_tree::child_iterator_t i3  = s.insert(s.child_begin(), 30);
  ic_set_tree::child_iterator_t i11  = s.insert(i1, 110);
  ic_set_tree::child_iterator_t i12  = s.insert(i1, 120);
  ic_set_tree::child_iterator_t i13  = s.insert(i1, 130);
  ic_set_tree::child_iterator_t i21  = s.insert(i2, 210);
  ic_set_tree::child_iterator_t i22  = s.insert(i2, 220);
  ic_set_tree::child_iterator_t i23  = s.insert(i2, 230);
  ic_set_tree::child_iterator_t i31  = s.insert(i3, 310);
  ic_set_tree::child_iterator_t i32  = s.insert(i3, 320);
  ic_set_tree::child_iterator_t i33  = s.insert(i3, 330);

  UF_TEST_EQUAL(s.count(), 3*3+3);
  UF_TEST_EQUAL(s.count(), s.size());
  UF_TEST(!s.empty());
  UF_TEST_EQUAL(my_instance_counted_t::instances(), 3*3+3 + 1); // +1 for root node
  s.clear();
  UF_TEST_EQUAL(s.count(), 0);
  UF_TEST_EQUAL(s.count(), s.size());
  UF_TEST( s.empty());
  UF_TEST_EQUAL(my_instance_counted_t::instances(), 1);
}

static void
test_set_tree_find()
{
  tracing_allocator_t alloc;
  int_set_tree s(0, &alloc, 1024);
  int_set_tree::child_iterator_t i0  = s.insert(s.child_begin(), 0);
  int_set_tree::child_iterator_t i01 = s.insert(i0, 10);
  int_set_tree::child_iterator_t i02 = s.insert(i0, 20);

  int_set_tree::child_iterator_t i01b = s.find(i0, 10);
  UF_TEST_EQUAL(i01b, i01);
  int_set_tree::child_iterator_t i03  = s.find(i0, 30);
  UF_TEST(i03.atend());
  int_set_tree::child_iterator_t i11  = s.find(i01, 10); // different parent
  UF_TEST(i11.atend());
}

static void
test_set_tree_child_count()
{
  tracing_allocator_t alloc;
  int_set_tree s(0, &alloc, 1024);

  int_set_tree::child_iterator_t i1  = s.insert(s.child_begin(), 10);
  int_set_tree::child_iterator_t i2  = s.insert(s.child_begin(), 20);
  int_set_tree::child_iterator_t i22 = s.insert(i2, 22);
  int_set_tree::child_iterator_t i21 = s.insert(i2, 21);
  int_set_tree::child_iterator_t i23 = s.insert(i2, 23);

  UF_TEST_EQUAL(s.child_count(s.child_begin()), 2);
  UF_TEST_EQUAL(s.child_count(i1), 0);
  UF_TEST( s.child_empty(i1));
  UF_TEST_EQUAL(s.child_count(i2), 3);
  UF_TEST(!s.child_empty(i2));
  UF_TEST_EQUAL(s.child_size(i2), s.child_count(i2)); // ensure to instantiate child_size() entity
}

static void
test_set_tree_iterator()
{
  tracing_allocator_t alloc;
  int_set_tree s(0, &alloc, 1024);

  int_set_tree::child_iterator_t i1   = s.insert(s.child_begin(), 100);
  int_set_tree::child_iterator_t i2   = s.insert(s.child_begin(), 200);
  int_set_tree::child_iterator_t i3   = s.insert(s.child_begin(), 300);
  int_set_tree::child_iterator_t i21  = s.insert(i2, 210);
  int_set_tree::child_iterator_t i22  = s.insert(i2, 220);
  int_set_tree::child_iterator_t i23  = s.insert(i2, 230);
  int_set_tree::child_iterator_t i231 = s.insert(i23, 231);
  int_set_tree::child_iterator_t i232 = s.insert(i23, 232);
  int_set_tree::child_iterator_t i233 = s.insert(i23, 233);

  int_set_tree::iterator i = s.begin();
  UF_TEST_EQUAL(*i, 100);
  UF_TEST_EQUAL(i.height(), 2); // root and first child
  ++i;
  UF_TEST_EQUAL(*i, 210);
  UF_TEST_EQUAL(i.height(), 3);
  ++i;
  UF_TEST_EQUAL(*i, 220);
  ++i;
  UF_TEST_EQUAL(*i, 231);
  UF_TEST_EQUAL(i.height(), 4);
  ++i;
  UF_TEST_EQUAL(*i, 232);
  ++i;
  UF_TEST_EQUAL(*i, 233);
  ++i;
  UF_TEST_EQUAL(*i, 230);
  ++i;
  UF_TEST_EQUAL(*i, 200);
  ++i;
  UF_TEST_EQUAL(*i, 300);
  ++i;
  UF_TEST_EQUAL(*i,   0);
  ++i;
  UF_TEST_EQUAL(i, s.end());

  int_set_tree::const_child_iterator_t i2chi = s.child_begin_for(i2);
  int_set_tree::const_child_iterator_t i2end = s.child_end_for(i2);
  UF_TEST_EQUAL(i2chi, i21);
  i2chi++;
  UF_TEST_EQUAL(i2chi, i22);
  i2chi++;
  UF_TEST_EQUAL(i2chi, i23);
  i2chi++;
  UF_TEST_EQUAL(i2chi, i2end);
}

static void
test_set_tree_empty_iterator()
{
  tracing_allocator_t alloc;
  int_set_tree s(0, &alloc, 1024);

  int_set_tree::const_iterator i = s.begin();
  UF_TEST(i != s.end());
  ++i;
  UF_TEST_EQUAL(i, s.end());
}

void test_set_tree_ticket()
{
  tracing_allocator_t alloc;
  int_set_tree s(0, &alloc, 1024);
  int_set_tree::const_child_iterator_t i0 = s.insert(s.child_begin(), 100);
  int_set_tree::ticket_t t = s.to_ticket(i0);
  int_set_tree::const_child_iterator_t i1 = s.to_child_iterator(t);
  UF_TEST_EQUAL(i0, i1);
  
  int_set_tree::ticket_t tend = s.to_ticket(s.child_end());
  UF_TEST_EQUAL(s.to_child_iterator(tend), s.child_end());
}

void test_set_tree()
{
  test_set_tree_hello();
  test_set_tree_insert_remove();
  test_set_tree_clear();
  test_set_tree_find();
  test_set_tree_child_count();
  test_set_tree_iterator();
  test_set_tree_empty_iterator();
  test_set_tree_ticket();
}


/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
