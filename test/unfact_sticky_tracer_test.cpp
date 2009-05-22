
#include <unfact/sticky.hpp>
#include <unfact/tick_tracer.hpp>
#include <unfact/extras/tick_tracing_annotation.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>
#include <vector>
#include <algorithm>
#include <string>

namespace uf = unfact;

void test_sticky_accumulation_hello()
{
  typedef uf::sticky_accumulation_t<int, 4> small_int_sticky_type;
  
  small_int_sticky_type s;
  s.trace( 4);
  s.trace( 8);
  s.trace(12);

  UF_TEST_EQUAL(s.total(), 24);
  UF_TEST_EQUAL(s.average(), 8);
  UF_TEST_EQUAL(s.samples(), 3);

  s.trace(5); // will reach modulo
  UF_TEST_EQUAL(s.total(), 5);
  UF_TEST_EQUAL(s.average(), 5);
  UF_TEST_EQUAL(s.samples(), 1);

  s.clear();
  UF_TEST_EQUAL(s.total(), 0);
  UF_TEST_EQUAL(s.average(), 0);
  UF_TEST_EQUAL(s.samples(), 0);
}

void test_tick_ops_hello()
{
  /* test only compilcation ... */
  typedef uf::default_tick_ops_t ops_type;
  ops_type::value_type x = ops_type::tick();
  ops_type::value_type y = ops_type::tick();
  ops_type::delta_type d = ops_type::distance(x, y);
  float ms = ops_type::to_milliseconds(d);
  ignore_variable(ms);
}

void test_tick_tracer_hello()
{
  tracing_allocator_t alloc;
  uf::accumulative_tick_tracer_t tr(&alloc);
  tr.trace(tr.root(), 10);

  {
	uf::accumulative_tick_scope_t s(&tr, tr.root());
  }
}

void test_tick_tracer_format()
{
  typedef uf::accumulative_tick_tracer_t tracer_type;
  tracing_allocator_t alloc;
  tracer_type tr(&alloc);
  
  tracer_type::ticket_type t0 = tr.push(tr.root(), "hello");
  tracer_type::ticket_type t1 = tr.push(tr.root(), "bye");
  
  tr.trace(tr.root(), 10.0f);
  tr.trace(t0, 20.0f);
  tr.trace(t1, 30.0f);
  tr.trace(t1, 40.0f);

  // TODO: format here
  char buf[256];
  uf::accumulative_tick_tracing_formatter_t f(&tr.tracer(), buf, 256, tr.root());
  UF_TEST_EQUAL("    35.0 (     2 times):bye", std::string(buf));
  f.increment();
  UF_TEST_EQUAL("    20.0 (     1 times):hello", std::string(buf));
  f.increment();
  UF_TEST_EQUAL("    10.0 (     1 times):", std::string(buf));
  f.increment();
  UF_TEST_EQUAL("", std::string(buf));
  UF_TEST(f.atend());

  tr.fill();
  uf::accumulative_tick_tracing_formatter_t f2(&tr.tracer(), buf, 256, tr.root());
  UF_TEST_EQUAL("     0.0 (     0 times):bye", std::string(buf));
}

namespace ufx = unfact::extras;

void test_cta_init_fini()
{
  typedef ufx::tick_tracing_annotation_t<12> annotation_type;
  typedef ufx::tracing_annotation_context_t< annotation_type > context_type;
  
  context_type ctx = { 0, {0} };

  ctx.init();

  ctx->chain().push("hello");
  ctx->chain().pop();

  {
	annotation_type::counting_scope_type s(&(ctx->chain()), "bye");
  }

  ctx.fini();
}

namespace 
{
	UFX_TICK_TRACE_DECLARE();
	UFX_TICK_TRACE_DEFINE();

	void test_cta_macros_count()
	{
		UFX_TICK_TRACE_INIT();
		UFX_TICK_TRACE_PUSH(hello);
		{
			UFX_TICK_TRACE_SCOPE_COUNT(howareyou);
			{
			  UFX_TICK_TRACE_DISJOIN_COUNT(fine);
			}
			UFX_TICK_TRACE_REPORT();
			UFX_TICK_TRACE_CLEAR_HERE();
		}
		UFX_TICK_TRACE_POP();
		UFX_TICK_TRACE_CLEAR();
		UFX_TICK_TRACE_FINI();
	}

	void test_cta_macros_nocount()
	{
		UFX_TICK_TRACE_INIT();
		UFX_TICK_TRACE_PUSH(hello);
		{
			UFX_TICK_TRACE_SCOPE(howareyou);
			UFX_TICK_TRACE_START();
			{
			  UFX_TICK_TRACE_DISJOIN(fine);
			}
			UFX_TICK_TRACE_STOP();
			UFX_TICK_TRACE_REPORT();
		}
		UFX_TICK_TRACE_POP();
		UFX_TICK_TRACE_CLEAR();
		UFX_TICK_TRACE_FINI();
	}
	
	void test_cta_macros_noinit()
	{
		//UFX_TICK_TRACE_INIT();
		UFX_TICK_TRACE_PUSH(hello);
		{
		  UFX_TICK_TRACE_SCOPE_COUNT(howareyou);
		  {
			UFX_TICK_TRACE_DISJOIN_COUNT(fine);
		  }
		}
		UFX_TICK_TRACE_POP();
		UFX_TICK_TRACE_CLEAR();
		//UFX_TICK_TRACE_FINI();
	}
}

void test_hello_tick_scope_set()
{
  tracing_allocator_t alloc;
  uf::accumulative_tick_tracer_t tr(&alloc);
  typedef uf::tick_scope_t<uf::accumulative_tick_tracer_t> scope_type;
  typedef uf::tree_set_t<scope_type> tick_scope_set_type;
  tick_scope_set_type tsset(&alloc);

  tsset.insert(scope_type::initializer_t(&tr, tr.root()));
  tsset.find(tr.root());
}

void test_sticky_tracer()
{
  test_sticky_accumulation_hello();
  test_tick_ops_hello();
  test_tick_tracer_hello();
  test_tick_tracer_format();
  test_cta_init_fini();
  test_cta_macros_count();
  test_cta_macros_nocount();
  test_cta_macros_noinit();
  test_hello_tick_scope_set();
}


/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
