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

#ifndef UNFACT_EXTRAS_TICK_TRACING_ANNOTATION_HPP
#define UNFACT_EXTRAS_TICK_TRACING_ANNOTATION_HPP

#include <unfact/tick_tracer.hpp>
#include <unfact/extras/base.hpp>
#include <unfact/extras/tracing_chain.hpp>

UNFACT_NAMESPACE_EXTRAS_BEGIN

/*
 * TODO: doc
 */
template<size_t StorageID, class Allocator=backdoor_allocator_t>
class tick_tracing_annotation_t
{
public:
	typedef Allocator allocator_type;
	typedef tick_tracing_annotation_t self_type;
	typedef accumulative_tick_tracer_t tracer_type;
	typedef typename tracer_type::ticket_type ticket_type;
	typedef tracing_chain_t<tracer_type, StorageID> chain_type;
	typedef typename chain_type::scope_t scope_type;
	typedef typename chain_type::disjoint_t disjoint_type;
	typedef tick_scope_t<tracer_type> tick_scope_type;
	typedef tick_scope_t<tracer_type, duration_while_tag_t> while_tick_scope_type;
  typedef tree_set_t<while_tick_scope_type> while_tick_scope_set_type;
	typedef typename while_tick_scope_type::initializer_t while_tick_scope_initializer_type;
 
	class counting_scope_t : public scope_type
	{
	public:
		counting_scope_t(chain_type* chain, const char* name)
			: scope_type(chain, name), 
				m_tick_scope(chain ? chain->tracer() : 0, chain ? chain->ticket() : 0) 
		{}
	private:
		tick_scope_type m_tick_scope;
	};

	class counting_disjoint_t : public disjoint_type
	{
	public:
		counting_disjoint_t(chain_type* chain, const char* name)
			: disjoint_type(chain, name), 
				m_tick_scope(chain ? chain->tracer() : 0, chain ? chain->ticket() : 0)
		{}
	private:
		tick_scope_type m_tick_scope;
	};

	typedef counting_scope_t counting_scope_type;
	typedef counting_disjoint_t counting_disjoint_type;

	tick_tracing_annotation_t()
		: m_tracer(&m_allocator), m_chain(&m_tracer),	m_countings(&m_allocator) {}
	
	tracer_type& tracer(){ return m_tracer; }
	chain_type& chain() { return m_chain; }
	const tracer_type& tracer() const { return m_tracer; }
	ticket_type root() const  { return m_tracer.root(); }
	void clear() { m_tracer.fill(); }
	void clear_here() { m_tracer.fill(m_chain.ticket()); }

	void start_counting()
	{
		m_countings.insert
			(while_tick_scope_initializer_type(&m_tracer, chain().ticket())); 
	}

	void stop_counting()
	{
		m_countings.remove(m_countings.find(chain().ticket()));
	}

private:
	allocator_type m_allocator;
	tracer_type m_tracer;
	chain_type m_chain;
	while_tick_scope_set_type m_countings;
};

template<size_t StorageID, class Allocator>
inline void
report_tick_tracing_annotation(const tick_tracing_annotation_t<StorageID, Allocator>& annot)
{
	char buf[256];
  unfact::accumulative_tick_tracing_formatter_t f(&annot.tracer().tracer(), buf, 256, annot.tracer().root());
	while (!f.atend()) {
		UF_TRACE((buf));
		f.increment();
	}
}

typedef tick_tracing_annotation_t<thead_local_id_tick_tracing_annotation,
																	backdoor_allocator_t> 
        default_tick_tracing_annotation_type;
typedef tracing_annotation_context_t<default_tick_tracing_annotation_type> 
        default_tick_tracing_annotation_context_t;
typedef default_tick_tracing_annotation_type::counting_scope_type
        default_tick_tracing_annotation_counting_scope_t;
typedef default_tick_tracing_annotation_type::counting_disjoint_type
        default_tick_tracing_annotation_counting_disjoint_t;
typedef default_tick_tracing_annotation_type::scope_type
        default_tick_tracing_annotation_scope_t;
typedef default_tick_tracing_annotation_type::disjoint_type
        default_tick_tracing_annotation_disjoint_t;

UNFACT_NAMESPACE_EXTRAS_END

#ifndef UFX_USE_NO_TICK_TRACE
# define UFX_TICK_TRACE_DEFINE_X(name) unfact::extras::default_tick_tracing_annotation_context_t name
# define UFX_TICK_TRACE_INIT_X(name) name.init()
# define UFX_TICK_TRACE_FINI_X(name) name.fini()
# define UFX_TICK_TRACE_SCOPE_COUNT_X(name, scope) unfact::extras::default_tick_tracing_annotation_counting_scope_t ufx_hta_scope_##scope(UFX_TICK_TRACE_CHAIN_X(name), #scope)
# define UFX_TICK_TRACE_SCOPE_COUNT_STR_X(name, var, scope) unfact::extras::default_tick_tracing_annotation_counting_scope_t ufx_hta_scope_##var(UFX_TICK_TRACE_CHAIN_X(name), scope)
# define UFX_TICK_TRACE_DISJOIN_COUNT_STR_X(name, var, scope) unfact::extras::default_tick_tracing_annotation_counting_disjoint_t ufx_hta_disjoin_##var(UFX_TICK_TRACE_CHAIN_X(name), scope)
# define UFX_TICK_TRACE_DISJOIN_COUNT_X(name, scope) unfact::extras::default_tick_tracing_annotation_counting_disjoint_t ufx_hta_disjoin_##scope(UFX_TICK_TRACE_CHAIN_X(name), #scope)
# define UFX_TICK_TRACE_SCOPE_X(name, scope) unfact::extras::default_tick_tracing_annotation_scope_t ufx_hta_scope_##scope(UFX_TICK_TRACE_CHAIN_X(name), #scope)
# define UFX_TICK_TRACE_SCOPE_STR_X(name, var, scope) unfact::extras::default_tick_tracing_annotation_scope_t ufx_hta_scope_##var(UFX_TICK_TRACE_CHAIN_X(name), scope)
# define UFX_TICK_TRACE_DISJOIN_STR_X(name, var, scope) unfact::extras::default_tick_tracing_annotation_disjoint_t ufx_hta_disjoin_##var(UFX_TICK_TRACE_CHAIN_X(name), scope)
# define UFX_TICK_TRACE_DISJOIN_X(name, scope) unfact::extras::default_tick_tracing_annotation_disjoint_t ufx_hta_disjoin_##scope(UFX_TICK_TRACE_CHAIN_X(name), #scope)
# define UFX_TICK_TRACE_PUSH_X(name, scope) if (name.good()) name->chain().push(#scope)
# define UFX_TICK_TRACE_PUSH_STR_X(name, scope) if (name.good()) name->chain().push(scope)
# define UFX_TICK_TRACE_POP_X(name) if (name.good()) name->chain().pop()
# define UFX_TICK_TRACE_CLEAR_X(name) if (name.good()) name->clear()
# define UFX_TICK_TRACE_CLEAR_HERE_X(name) if (name.good()) name->clear_here()
# define UFX_TICK_TRACE_TICKET_X(name) (name.good() ? name->chain().ticket() : 0)
# define UFX_TICK_TRACE_TRACER_X(name) name.self
# define UFX_TICK_TRACE_CHAIN_X(name) (name.good() ? &(name->chain()) : 0)
# define UFX_TICK_TRACE_START_X(name)  if (name.good()) name->start_counting()
# define UFX_TICK_TRACE_STOP_X(name) if (name.good()) name->stop_counting()
# define UFX_TICK_TRACE_DECLARE_X(name) extern unfact::extras::default_tick_tracing_annotation_context_t name
# define UFX_TICK_TRACE_REPORT_X(name) if (name.good()) report_tick_tracing_annotation(*(name.self))
#else
# define UFX_TICK_TRACE_DEFINE_X(name) ((void)0)
# define UFX_TICK_TRACE_INIT_X(name) ((void)0)
# define UFX_TICK_TRACE_FINI_X(name) ((void)0)
# define UFX_TICK_TRACE_SCOPE_COUNT_X(name, scope) ((void)0)
# define UFX_TICK_TRACE_SCOPE_COUNT_STR_X(name, var, scope) ((void)0)
# define UFX_TICK_TRACE_DISJOIN_COUNT_STR_X(name, var, scope) ((void)0)
# define UFX_TICK_TRACE_DISJOIN_COUNT_X(name, scope) ((void)0)
# define UFX_TICK_TRACE_SCOPE_X(name, scope) ((void)0)
# define UFX_TICK_TRACE_SCOPE_STR_X(name, var, scope) ((void)0)
# define UFX_TICK_TRACE_DISJOIN_STR_X(name, var, scope) ((void)0)
# define UFX_TICK_TRACE_DISJOIN_X(name, scope) ((void)0)
# define UFX_TICK_TRACE_PUSH_X(name, scope) ((void)0)
# define UFX_TICK_TRACE_PUSH_STR_X(name, scope) ((void)0)
# define UFX_TICK_TRACE_POP_X(name) ((void)0)
# define UFX_TICK_TRACE_CLEAR_X(name) ((void)0)
# define UFX_TICK_TRACE_CLEAR_HERE_X(name) ((void)0)
# define UFX_TICK_TRACE_TICKET_X(name) (0)
# define UFX_TICK_TRACE_TRACER_X(name) (0)
# define UFX_TICK_TRACE_CHAIN_X(name) (0)
# define UFX_TICK_TRACE_START_X(name)  ((void)0)
# define UFX_TICK_TRACE_STOP_X(name) ((void)0)
# define UFX_TICK_TRACE_DECLARE_X(name) ((void)0)
# define UFX_TICK_TRACE_REPORT_X(name) ((void)0)
#endif

#define UFX_TICK_TRACE_NAME g_ufx_tta_context
#define UFX_TICK_TRACE_DECLARE() UFX_TICK_TRACE_DECLARE_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_DEFINE() UFX_TICK_TRACE_DEFINE_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_INIT() UFX_TICK_TRACE_INIT_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_FINI() UFX_TICK_TRACE_FINI_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_SCOPE_COUNT(scope) UFX_TICK_TRACE_SCOPE_COUNT_X(UFX_TICK_TRACE_NAME, scope)
#define UFX_TICK_TRACE_SCOPE_COUNT_STR(var, scope) UFX_TICK_TRACE_SCOPE_COUNT_STR_X(UFX_TICK_TRACE_NAME, var, scope)
#define UFX_TICK_TRACE_DISJOIN_COUNT(disjoin) UFX_TICK_TRACE_DISJOIN_COUNT_X(UFX_TICK_TRACE_NAME, disjoin)
#define UFX_TICK_TRACE_DISJOIN_COUNT_STR(var, disjoin) UFX_TICK_TRACE_DISJOIN_COUNT_STR_X(UFX_TICK_TRACE_NAME, var, disjoin)
#define UFX_TICK_TRACE_SCOPE(scope) UFX_TICK_TRACE_SCOPE_X(UFX_TICK_TRACE_NAME, scope)
#define UFX_TICK_TRACE_SCOPE_STR(var, scope) UFX_TICK_TRACE_SCOPE_STR_X(UFX_TICK_TRACE_NAME, var, scope)
#define UFX_TICK_TRACE_DISJOIN(disjoin) UFX_TICK_TRACE_DISJOIN_X(UFX_TICK_TRACE_NAME, disjoin)
#define UFX_TICK_TRACE_DISJOIN_STR(var, disjoin) UFX_TICK_TRACE_DISJOIN_STR_X(UFX_TICK_TRACE_NAME, var, disjoin)
#define UFX_TICK_TRACE_PUSH(scope) UFX_TICK_TRACE_PUSH_X(UFX_TICK_TRACE_NAME, scope)
#define UFX_TICK_TRACE_PUSH_STR(scope) UFX_TICK_TRACE_PUSH_STR_X(UFX_TICK_TRACE_NAME, (scope))
#define UFX_TICK_TRACE_POP() UFX_TICK_TRACE_POP_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_CLEAR() UFX_TICK_TRACE_CLEAR_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_CLEAR_HERE() UFX_TICK_TRACE_CLEAR_HERE_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_TICKET() UFX_TICK_TRACE_TICKET_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_TRACER() UFX_TICK_TRACE_TRACER_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_START() UFX_TICK_TRACE_START_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_STOP() UFX_TICK_TRACE_STOP_X(UFX_TICK_TRACE_NAME)
#define UFX_TICK_TRACE_REPORT() UFX_TICK_TRACE_REPORT_X(UFX_TICK_TRACE_NAME)

#endif//UNFACT_EXTRAS_TICK_TRACING_ANNOTATION_HPP

/* -*-
 Local Variables:
 mode: c++
 c-tab-always-indent: t
 c-indent-level: 2
 c-basic-offset: 2
 tab-width: 2
 End:
 -*- */
