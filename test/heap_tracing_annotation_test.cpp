
#include <unfact/extras/heap_tracing_annotation.hpp>
#include <test/memory_support.hpp>
#include <test/tracer_support.hpp>
#include <test/unit.hpp>

namespace ufx = unfact::extras;

void test_hta_hello()
{
	ufx::heap_tracing_annotation_t<10,ufx::backdoor_allocator_t> a;
}

void test_hta_init_fini()
{
	typedef ufx::heap_tracing_annotation_t<11> annotation_type;
	typedef ufx::tracing_annotation_context_t< annotation_type > context_type;
	context_type ctx = { 0, {0} };

	UF_TEST_EQUAL(ctx.self, 0);
	ctx.init();
	UF_TEST(ctx.self);

	UF_TEST( ctx->chain().empty());
	UF_TEST_EQUAL("", qualified_tracing_name(ctx.self->tracer(), ctx->chain().ticket()));
	ctx->chain().push("hello");
	UF_TEST(!ctx->chain().empty());
	UF_TEST_EQUAL("hello", qualified_tracing_name(ctx.self->tracer(), ctx->chain().ticket()));
	ctx->chain().push("howau");
	UF_TEST(!ctx->chain().empty());
	UF_TEST_EQUAL("hello.howau", qualified_tracing_name(ctx.self->tracer(), ctx->chain().ticket()));
	{
		annotation_type::scope_type s(&(ctx.self->chain()), "fine");
		UF_TEST(!ctx->chain().empty());
		UF_TEST_EQUAL("hello.howau.fine", qualified_tracing_name(ctx.self->tracer(), ctx->chain().ticket()));
		{
			annotation_type::disjoint_type ds(&(ctx.self->chain()), "btw");
			UF_TEST_EQUAL("btw", qualified_tracing_name(ctx.self->tracer(), ctx->chain().ticket()));
		}
		// should be restored.
		UF_TEST_EQUAL("hello.howau.fine", qualified_tracing_name(ctx.self->tracer(), ctx->chain().ticket()));
	}
	ctx->chain().pop();
	UF_TEST(!ctx->chain().empty());
	UF_TEST_EQUAL("hello", qualified_tracing_name(ctx.self->tracer(), ctx->chain().ticket()));
	ctx->chain().pop();
	UF_TEST( ctx->chain().empty());
	UF_TEST_EQUAL("", qualified_tracing_name(ctx.self->tracer(), ctx->chain().ticket()));
	
	ctx.fini();
	UF_TEST_EQUAL(ctx.self, 0);
}

namespace 
{
	UFX_HEAP_TRACE_DECLARE();
	UFX_HEAP_TRACE_DEFINE();

	void test_hta_macros()
	{
		UFX_HEAP_TRACE_INIT();
		UFX_HEAP_TRACE_PUSH(hello);
		{
			UFX_HEAP_TRACE_SCOPE(howareyou);
			{
			  UFX_HEAP_TRACE_DISJOIN(fine);
			}
		}
		UFX_HEAP_TRACE_POP();
		UFX_HEAP_TRACE_FINI();
	}
	
	void test_hta_macros_noinit()
	{
		//UFX_HEAP_TRACE_INIT();
		UFX_HEAP_TRACE_PUSH(hello);
		{
			UFX_HEAP_TRACE_SCOPE(howareyou);
		}
		UFX_HEAP_TRACE_POP();
		//UFX_HEAP_TRACE_FINI();
	}
}

void test_heap_tracing_annotation()
{
  test_hta_hello();
	test_hta_init_fini();
	test_hta_macros();
	test_hta_macros_noinit();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   tab-width: 2
   End:
   -*- */
