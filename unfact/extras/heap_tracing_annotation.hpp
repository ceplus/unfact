/*
 * Copyright (c) 2008 Community Engine Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef UNFACT_EXTRAS_HEAP_TRACING_ANNOTATION_HPP
#define UNFACT_EXTRAS_HEAP_TRACING_ANNOTATION_HPP

#include <unfact/heap_tracer.hpp>
#include <unfact/extras/base.hpp>
#include <unfact/extras/tracing_chain.hpp>

UNFACT_NAMESPACE_EXTRAS_BEGIN

/*
 * TODO: doc
 */
template<size_t StorageID, class Allocator=backdoor_allocator_t>
class heap_tracing_annotation_t
{
public:
	typedef Allocator allocator_type;
	typedef heap_tracing_annotation_t self_type;
	typedef accumulative_heap_tracer_t tracer_type;
	typedef typename tracer_type::ticket_type ticket_type;
	typedef tracing_chain_t<tracer_type, StorageID> chain_type;
	typedef typename chain_type::scope_t scope_type;
	typedef typename chain_type::disjoint_t disjoint_type;

	heap_tracing_annotation_t() : m_tracer(&m_allocator), m_chain(&m_tracer) {}
	
	tracer_type& tracer(){ return m_tracer; }
	chain_type& chain() { return m_chain; }
	const tracer_type& tracer() const { return m_tracer; }
	ticket_type root() const  { return m_tracer.root(); }

  void* trace_allocated(void* ptr, size_t size)
  {
		return m_tracer.trace_allocated(chain().ticket(), reinterpret_cast<byte_t*>(ptr), size);
	}

  void trace_deallocated(void* ptr)
  {
		return m_tracer.trace_deallocated(reinterpret_cast<byte_t*>(ptr));
	}

	void report(const char* file, int line)
	{
		char buf[128];
		accumulative_heap_tracing_formatter_t formatter(&(m_tracer.tracer()), buf, 128, root());
		while (!formatter.atend()) {
			UF_TRACE_X(file, line, (buf));
			formatter.increment();
		}
	}

	void assert_no_leakage(const char* file, int line)
	{
		if (0 < m_tracer.size()) {
			UF_TRACE_X(file, line, ("Found Memory Leak!:"));
			report(file, line);
			UF_ASSERT_X(file, line, false);
		}
	}

private:
	allocator_type m_allocator;
	tracer_type m_tracer;
	chain_type m_chain;
};

typedef heap_tracing_annotation_t<thead_local_id_heap_tracing_annotation,
																	backdoor_allocator_t> 
        default_heap_tracing_annotation_type;
typedef tracing_annotation_context_t<default_heap_tracing_annotation_type> 
        default_heap_tracing_annotation_context_t;
typedef default_heap_tracing_annotation_type::scope_type
        default_heap_tracing_annotation_scope_t;
typedef default_heap_tracing_annotation_type::disjoint_type
        default_heap_tracing_annotation_disjoint_t;

UNFACT_NAMESPACE_EXTRAS_END

#ifndef UFX_USE_NO_HEAP_TRACE
# define UFX_HEAP_TRACE_DEFINE_X(name) unfact::extras::default_heap_tracing_annotation_context_t name
# define UFX_HEAP_TRACE_INIT_X(name) name.init()
# define UFX_HEAP_TRACE_FINI_X(name) name.fini()
# define UFX_HEAP_TRACE_TRACER_X(name) name.self
# define UFX_HEAP_TRACE_CHAIN_X(name) (name.good() ? &(name->chain()) : 0)
# define UFX_HEAP_TRACE_SCOPE_X(name, scope) unfact::extras::default_heap_tracing_annotation_scope_t ufx_hta_scope_##scope(UFX_HEAP_TRACE_CHAIN_X(name), #scope)
# define UFX_HEAP_TRACE_DISJOIN_STR_X(name, var, scope) unfact::extras::default_heap_tracing_annotation_disjoint_t ufx_hta_disjoin_##var(UFX_HEAP_TRACE_CHAIN_X(name), scope)
# define UFX_HEAP_TRACE_DISJOIN_X(name, scope) unfact::extras::default_heap_tracing_annotation_disjoint_t ufx_hta_disjoin_##scope(UFX_HEAP_TRACE_CHAIN_X(name), #scope)
# define UFX_HEAP_TRACE_SCOPE_STR_X(name, var, scope) unfact::extras::default_heap_tracing_annotation_scope_t ufx_hta_scope_##var(UFX_HEAP_TRACE_CHAIN_X(name), scope)
# define UFX_HEAP_TRACE_PUSH_X(name, scope) if (name.good()) name->chain().push(#scope)
# define UFX_HEAP_TRACE_PUSH_STR_X(name, scope) if (name.good()) name->chain().push(scope)
# define UFX_HEAP_TRACE_POP_X(name) if (name.good()) name->chain().pop()
# define UFX_HEAP_TRACE_TICKET_X(name) (name.good() ? name->chain().ticket() : 0)
# define UFX_HEAP_TRACE_DECLARE_X(name) extern unfact::extras::default_heap_tracing_annotation_context_t name
# define UFX_HEAP_TRACE_MALLOC_X(name, ptr, sz)  ((name.good()) ? name->trace_allocated(ptr, sz) : ptr)
# define UFX_HEAP_TRACE_FREE_X(name, ptr) if (name.good()) name->trace_deallocated(ptr)
# define UFX_HEAP_TRACE_REPORT_X(name) if (name.good()) name->report(__FILE__, __LINE__)
# define UFX_HEAP_TRACE_ASSERT_NO_LEAKAGE_X(name) if (name.good()) name->assert_no_leakage(__FILE__, __LINE__)
#else
# define UFX_HEAP_TRACE_DEFINE_X(name) ((void)0)
# define UFX_HEAP_TRACE_INIT_X(name) ((void)0)
# define UFX_HEAP_TRACE_FINI_X(name) ((void)0)
# define UFX_HEAP_TRACE_SCOPE_X(name, scope) ((void)0)
# define UFX_HEAP_TRACE_SCOPE_STR_X(name, var, scope) ((void)0)
# define UFX_HEAP_TRACE_DISJOIN_STR_X(name, var, scope) ((void)0)
# define UFX_HEAP_TRACE_DISJOIN_X(name, scope) ((void)0)
# define UFX_HEAP_TRACE_PUSH_X(name, scope) ((void)0)
# define UFX_HEAP_TRACE_PUSH_STR_X(name, scope) ((void)0)
# define UFX_HEAP_TRACE_POP_X(name) ((void)0)
# define UFX_HEAP_TRACE_TICKET_X(name) (0)
# define UFX_HEAP_TRACE_DECLARE_X(name) ((void)0)
# define UFX_HEAP_TRACE_MALLOC_X(name, ptr, sz) (ptr)
# define UFX_HEAP_TRACE_FREE_X(name, ptr) ((void)0)
# define UFX_HEAP_TRACE_TRACER_X(name) (0)
# define UFX_HEAP_TRACE_REPORT_X(name) ((void)0)
# define UFX_HEAP_TRACE_ASSERT_NO_LEAKAGE_X(name) ((void)0)
#endif

#define UFX_HEAP_TRACE_NAME g_ufx_hta_context
#define UFX_HEAP_TRACE_DECLARE() UFX_HEAP_TRACE_DECLARE_X(UFX_HEAP_TRACE_NAME)
#define UFX_HEAP_TRACE_DEFINE() UFX_HEAP_TRACE_DEFINE_X(UFX_HEAP_TRACE_NAME)
#define UFX_HEAP_TRACE_INIT() UFX_HEAP_TRACE_INIT_X(UFX_HEAP_TRACE_NAME)
#define UFX_HEAP_TRACE_FINI() UFX_HEAP_TRACE_FINI_X(UFX_HEAP_TRACE_NAME)
#define UFX_HEAP_TRACE_SCOPE(scope) UFX_HEAP_TRACE_SCOPE_X(UFX_HEAP_TRACE_NAME, scope)
#define UFX_HEAP_TRACE_SCOPE_STR(var, scope) UFX_HEAP_TRACE_SCOPE_STR_X(UFX_HEAP_TRACE_NAME, var, scope)
#define UFX_HEAP_TRACE_DISJOIN(disjoin) UFX_HEAP_TRACE_DISJOIN_X(UFX_HEAP_TRACE_NAME, disjoin)
#define UFX_HEAP_TRACE_DISJOIN_STR(var, disjoin) UFX_HEAP_TRACE_DISJOIN_STR_X(UFX_HEAP_TRACE_NAME, var, disjoin)
#define UFX_HEAP_TRACE_PUSH(scope) UFX_HEAP_TRACE_PUSH_X(UFX_HEAP_TRACE_NAME, scope)
#define UFX_HEAP_TRACE_PUSH_STR(scope) UFX_HEAP_TRACE_PUSH_STR_X(UFX_HEAP_TRACE_NAME, (scope))
#define UFX_HEAP_TRACE_POP() UFX_HEAP_TRACE_POP_X(UFX_HEAP_TRACE_NAME)
#define UFX_HEAP_TRACE_TICKET() UFX_HEAP_TRACE_TICKET_X(UFX_HEAP_TRACE_NAME)
#define UFX_HEAP_TRACE_MALLOC(ptr, sz)  UFX_HEAP_TRACE_MALLOC_X(UFX_HEAP_TRACE_NAME, ptr, sz) 
#define UFX_HEAP_TRACE_FREE(ptr) UFX_HEAP_TRACE_FREE_X(UFX_HEAP_TRACE_NAME, ptr) 
#define UFX_HEAP_TRACE_TRACER() UFX_HEAP_TRACE_TRACER_X(UFX_HEAP_TRACE_NAME)
#define UFX_HEAP_TRACE_REPORT() UFX_HEAP_TRACE_REPORT_X(UFX_HEAP_TRACE_NAME)
#define UFX_HEAP_TRACE_ASSERT_NO_LEAKAGE() UFX_HEAP_TRACE_ASSERT_NO_LEAKAGE_X(UFX_HEAP_TRACE_NAME)

#endif//UNFACT_EXTRAS_HEAP_TRACING_ANNOTATION_HPP

/* -*-
 Local Variables:
 mode: c++
 c-tab-always-indent: t
 c-indent-level: 2
 c-basic-offset: 2
 tab-width: 2
 End:
 -*- */
