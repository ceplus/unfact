
#include <unfact/heap_tracer.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>
#include <vector>
#include <algorithm>
#include <string>

namespace uf = unfact;
typedef uf::accumulative_heap_tracer_t tracer_type;

void test_heap_tracer_hello()
{
  /* just instantiate */
  typedef tracer_type tracer_type;
  tracing_allocator_t alloc;
  tracer_type t(&alloc);
  tracer_type::heap_iterator hi = t.heap_begin();
  tracer_type::heap_iterator hj = t.heap_begin();

  tracer_type::ticket_type t0 = t.root();
  tracer_type::ticket_type t1 = t.push(t0, "hello");
  tracer_type::ticket_type t2 = t.pop(t1);
  t.name_of(t0);
  t.at(t0);

}

void test_heap_tracer_trace()
{
  tracing_allocator_t alloc;
  tracer_type tr(&alloc);
  
  tracer_type::ticket_type t0 = tr.push(tr.root(), "hello");
  tracer_type::ticket_type t1 = tr.push(t0, "howau");
  tracer_type::ticket_type t2 = tr.push(t0, "bye");

  uf::byte_t heap01 = 0;
  uf::byte_t heap03 = 0;
  uf::byte_t heap05 = 0;
  uf::byte_t heap07 = 0;
  uf::byte_t heap11 = 0;
  uf::byte_t heap13 = 0;

  tr.trace_allocated(tr.root(), &heap01, 10);
  tr.trace_allocated(tr.root(), &heap03, 30);
  tr.trace_allocated(t0, &heap05,  40);
  tr.trace_allocated(t0, &heap07,  70);
  tr.trace_allocated(t1, &heap11, 110);
  tr.trace_allocated(t2, &heap13, 130);

  UF_TEST_EQUAL(tr.at(tr.root()).final(), 10+30);
  UF_TEST_EQUAL(tr.at(t0).final(), 40+70);
  UF_TEST_EQUAL(tr.at(t1).final(), 110);
  UF_TEST_EQUAL(tr.at(t2).final(), 130);
  UF_TEST_EQUAL(tr.size(), 390);

  tr.trace_deallocated(&heap07);
  UF_TEST_EQUAL(tr.at(t0).final(), 40);

  tr.trace_deallocated(&heap11);
  UF_TEST_EQUAL(tr.at(t1).final(),  0);

  tr.trace_deallocated(&heap05);
  UF_TEST_EQUAL(tr.at(t0).final(),  0);

  tr.trace_deallocated(&heap13);
  UF_TEST_EQUAL(tr.at(t2).final(),  0);

  tr.trace_deallocated(&heap01);
  UF_TEST_EQUAL(tr.at(tr.root()).final(), 30);
  tr.trace_deallocated(&heap03);
  UF_TEST_EQUAL(tr.at(tr.root()).final(),  0);

  UF_TEST_EQUAL(tr.size(), 0);
}

namespace {
  struct typical_heap_setup_t
  {
	typical_heap_setup_t()
	  : tr(&alloc)
	{
	  h1 = 0;
	  h3 = 0;
	  h5 = 0;
	  h7 = 0;

	  tr.trace_allocated(tr.root(), &h1, 10);

	  t0 = tr.push(tr.root(), "hello");
	  tr.trace_allocated(t0, &h3, 30);

	  t1 = tr.push(t0, "howau");
	  tr.trace_allocated(t1, &h5, 50);

	  t2 = tr.push(t0, "imfine");
	  tr.trace_allocated(t2, &h7, 70);
	}

	tracing_allocator_t alloc;
	tracer_type tr;
	uf::byte_t h1;
	uf::byte_t h3;
	uf::byte_t h5;
	uf::byte_t h7;
	tracer_type::ticket_type t0;
	tracer_type::ticket_type t1;
	tracer_type::ticket_type t2;
  };
}

void test_heap_tracer_count()
{
  typical_heap_setup_t setup;

  UF_TEST_EQUAL(accumulation_count(setup.tr.tracer(), setup.tr.root()), 10 + 30 + 50 + 70);
  UF_TEST_EQUAL(accumulation_count(setup.tr.tracer(), setup.t0), 30 + 50 + 70);
  UF_TEST_EQUAL(accumulation_count(setup.tr.tracer(), setup.t1), 50);
  UF_TEST_EQUAL(accumulation_count(setup.tr.tracer(), setup.t2), 70);
}

void test_heap_tracing_formatter_hello()
{
  char buf[128];
  char testbuf[128];
  typical_heap_setup_t setup;
  uf::accumulation_formatter_t<tracer_type::tracer_type>
	f1(&(setup.tr.tracer()), buf, 128, setup.tr.root());

  size_t w = 0;
  f1.format_name(setup.tr.root(), testbuf, 128, &w);
  std::string nr = "";
  UF_TEST_EQUAL(nr, testbuf);
  UF_TEST_EQUAL(nr.size(), w);

  f1.format_name(setup.t0, testbuf, 128, &w);
  std::string n0 = "hello";
  UF_TEST_EQUAL(n0, testbuf);
  UF_TEST_EQUAL(n0.size(), w);

  f1.format_name(setup.t1, testbuf, 128, &w);
  std::string n1 = "hello.howau";
  UF_TEST_EQUAL(n1, testbuf);
  UF_TEST_EQUAL(n1.size(), w);

  f1.format_name(setup.t2, testbuf, 128, &w);
  std::string n2 = "hello.imfine";
  UF_TEST_EQUAL(n2, testbuf);
  UF_TEST_EQUAL(n2.size(), w);

  f1.format_name(setup.t1, testbuf, 1, &w);
  UF_TEST_EQUAL(std::string(""), testbuf);
  f1.format_name(setup.t1, testbuf, 4, &w);
  UF_TEST_EQUAL(std::string(""), testbuf);
  f1.format_name(setup.t1, testbuf, 6, &w);
  UF_TEST_EQUAL(std::string("hello"), testbuf);
  f1.format_name(setup.t1, testbuf, 7, &w);
  UF_TEST_EQUAL(std::string("hello."), testbuf);
  
  UF_TEST_EQUAL(f1.c_str(), std::string("       50:hello.howau"));
  UF_TEST(!f1.atend());

  f1.increment();
  UF_TEST_EQUAL(f1.c_str(), std::string("       70:hello.imfine"));
  UF_TEST(!f1.atend());
  
  f1.increment();
  UF_TEST_EQUAL(f1.c_str(), std::string("      150:hello"));
  UF_TEST(!f1.atend());

  f1.increment();
  UF_TEST_EQUAL(f1.c_str(), std::string("      160:"));
  UF_TEST(!f1.atend());

  f1.increment();
  UF_TEST(f1.atend());
  UF_TEST_EQUAL(f1.c_str(), std::string(""));


  // formatting removed should be skipped
  setup.tr.trace_deallocated(&(setup.h7));
  uf::accumulation_formatter_t<tracer_type::tracer_type>
	f2(&(setup.tr.tracer()), buf, 128, setup.tr.root());

  UF_TEST_EQUAL(f2.c_str(), std::string("       50:hello.howau"));
  f2.increment();
  UF_TEST_EQUAL(f2.c_str(), std::string("       80:hello"));
  f2.increment();
  UF_TEST_EQUAL(f2.c_str(), std::string("       90:"));
  UF_TEST(!f2.atend());
  f2.increment();
  UF_TEST( f2.atend());
  UF_TEST_EQUAL(f2.c_str(), std::string(""));

}

void test_heap_tracer()
{
  test_heap_tracer_hello();
  test_heap_tracer_trace();
  test_heap_tracer_count();
  test_heap_tracing_formatter_hello();
}


/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
