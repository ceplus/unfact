
#include <unfact/tree_tracer.hpp>
#include <test/memory_support.hpp>
#include <test/tracer_support.hpp>
#include <test/unit.hpp>
#include <vector>
#include <algorithm>

typedef unfact::tree_tracer_t<int> int_tree_tracer_type;

void test_tree_tracer_hello()
{
  tracing_allocator_t alloc;
  int_tree_tracer_type trac(&alloc);

  int_tree_tracer_type::ticket_type t0 = trac.push(trac.root(), "hello");
  UF_TEST_EQUAL(trac.name_of(t0), "hello");
  trac.at(t0) = 100;
  UF_TEST_EQUAL(trac.at(t0), 100);

  UF_TEST_EQUAL(qualified_tracing_name(trac, t0), "hello");
}

void test_tree_tracer_at()
{
  tracing_allocator_t alloc;
  int_tree_tracer_type trac(&alloc);

  int_tree_tracer_type::ticket_type hello  = trac.push(trac.root(), "hello");
  int_tree_tracer_type::ticket_type hello2 = trac.push(trac.root(), "hello");
  int_tree_tracer_type::ticket_type bye    = trac.push(trac.root(), "bye");

	trac.at(hello) = 7;
	trac.at(bye)   = 6;

  UF_TEST_EQUAL(7, trac.at(hello));
  UF_TEST_EQUAL(7, trac.at(hello2));
  UF_TEST_EQUAL(6, trac.at(bye));

  trac.at(hello2) = 17;
  UF_TEST_EQUAL(17, trac.at(hello));
  UF_TEST_EQUAL(17, trac.at(hello2));
  UF_TEST_EQUAL( 6, trac.at(bye));


}

void test_tree_tracer_fill()
{
  tracing_allocator_t alloc;
  int_tree_tracer_type trac(&alloc);

  int_tree_tracer_type::ticket_type t1 = trac.push(trac.root(), "hello");
  int_tree_tracer_type::ticket_type t2 = trac.push(t1, "bye");

	trac.at(trac.root()) = 10;
  trac.at(t1) = 20;
  trac.at(t2) = 30;
  UF_TEST_EQUAL(10, trac.at(trac.root()));
  UF_TEST_EQUAL(20, trac.at(t1));
  UF_TEST_EQUAL(30, trac.at(t2));

	trac.fill(0);
  UF_TEST_EQUAL(0, trac.at(trac.root()));
  UF_TEST_EQUAL(0, trac.at(t1));
  UF_TEST_EQUAL(0, trac.at(t2));
}

void test_tree_tracer_push()
{
  tracing_allocator_t alloc;
  int_tree_tracer_type trac(&alloc);

  int_tree_tracer_type::ticket_type hello  = trac.push(trac.root(), "hello");
  int_tree_tracer_type::ticket_type hello2 = trac.push(trac.root(), "hello");
  int_tree_tracer_type::ticket_type bye    = trac.push(trac.root(), "bye");
  int_tree_tracer_type::ticket_type hellobye = trac.push(hello, "bye");

  UF_TEST_EQUAL(hello, hello2);
  UF_TEST_EQUAL(qualified_tracing_name(trac, hello), "hello");
  UF_TEST_EQUAL(qualified_tracing_name(trac, bye), "bye");
  UF_TEST_EQUAL(qualified_tracing_name(trac, hellobye), "hello.bye");
}

void test_tree_tracer_pop()
{
  tracing_allocator_t alloc;
  int_tree_tracer_type trac(&alloc);
  int_tree_tracer_type::ticket_type foo = trac.push(trac.root(), "foo");
  int_tree_tracer_type::ticket_type bar = trac.push(foo, "bar");
  int_tree_tracer_type::ticket_type baz = trac.push(bar, "baz");

  UF_TEST_EQUAL(qualified_tracing_name(trac, foo), "foo");
  UF_TEST_EQUAL(qualified_tracing_name(trac, bar), "foo.bar");
  UF_TEST_EQUAL(qualified_tracing_name(trac, baz), "foo.bar.baz");

  int_tree_tracer_type::ticket_type pbaz = trac.pop(baz);
  int_tree_tracer_type::ticket_type pbar = trac.pop(bar);
  int_tree_tracer_type::ticket_type pfoo = trac.pop(foo);

  UF_TEST_EQUAL(pbaz, bar);
  UF_TEST_EQUAL(pbar, foo);
  UF_TEST_EQUAL(pfoo, trac.root());
}

void test_tracing()
{
  test_tree_tracer_hello();
	test_tree_tracer_fill();
	test_tree_tracer_at();
  test_tree_tracer_push();
  test_tree_tracer_pop();
}


/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   tab-width:2
   End:
   -*- */
