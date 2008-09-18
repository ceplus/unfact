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

#ifndef UNFACT_EXTRAS_TRACING_CHAIN_HPP
#define UNFACT_EXTRAS_TRACING_CHAIN_HPP

#include <unfact/extras/base.hpp>
#include <unfact/extras/thread_local.hpp>

UNFACT_NAMESPACE_EXTRAS_BEGIN

/*
 * A scope chain for tracers, relying on the thread local stack.
 *
 * The stack implementation only has single "ticket" as its stack top, instead of keeping all stack item list.
 * Because ticket is a set_tree node and have its parent,
 * we can virtually "pop" the stack item replacing current top with its parnet, 
 * and "push" item replacing it with one of its children.
 *
 * Stack top is kept in thread local storage. So tracing_chain is thread-safe 
 * as the ticket container (set_tree) is.
 * 
 */
template<class Tracer, size_t StorageID>
class tracing_chain_t
{
public:
	typedef Tracer tracer_type;
	typedef tracing_chain_t self_type;
	typedef typename Tracer::ticket_type ticket_type;
	typedef typename default_thread_local_t<StorageID>::type thread_local_type;

	class scope_t
	{
	public:
		scope_t(self_type* self, const char* name) : m_self(self) { if (self) { self->push(name); } }
		~scope_t() { if (m_self) { m_self->pop(); } }
	private:
		scope_t(const scope_t& that);
		const scope_t& operator=(const scope_t& that);
	private:
		self_type* m_self;
	};

	class disjoint_t
	{
	public:
		disjoint_t(self_type* self, const char* name)
			: m_self(self), m_last(self ? self->disjoin(name) : 0) {}
		~disjoint_t() { if (m_self) { m_self->set_top(m_last); } }
	private:
		disjoint_t(const disjoint_t& that);
		const disjoint_t& operator=(const disjoint_t& that);
	private:
		self_type* m_self;
		ticket_type m_last;
	};

	tracing_chain_t(tracer_type* tracer) : m_tracer(tracer) {}

	ticket_type top() const
	{
		return reinterpret_cast<ticket_type>(m_local.get());
	}

	ticket_type ticket() const
	{
		typename thread_local_type::value_type v = m_local.get();
		if (v) {
			return reinterpret_cast<ticket_type>(v);
		} else {
			return m_tracer->root();
		}
	}

	ticket_type root() const  { return m_tracer->root(); }

	void set_top(ticket_type t)
	{
		m_local.set(reinterpret_cast<typename thread_local_type::value_type>(t));
	}

	void clear_top()
	{
		m_local.clear();
	}

	void push(const char* scope)
	{
		ticket_type last = top();
		ticket_type t = 0;
		if (last) {
			t = m_tracer->push(last, scope);
		} else {
			t = m_tracer->push(m_tracer->root(), scope);
		}
		set_top(t);
	}

	void pop()
	{
		ticket_type last = top();
		ticket_type t = m_tracer->pop(last);
		if (t == m_tracer->root()) {
			clear_top();
		} else {
			set_top(t);
		}
	}

	ticket_type disjoin(const char* scope)
	{
		ticket_type last = top();
		ticket_type t = m_tracer->push(m_tracer->root(), scope);
		set_top(t);
		return last;
	}

	bool empty() const { return 0 == m_local.get(); }

	tracer_type* tracer() const { return m_tracer; }

private:
	thread_local_type m_local;
	tracer_type* m_tracer;	
};


/*
 * helper to instantiate an annotation object
 * To make instantiation explicit, we put this POD object to global 
 * instead of directly instantiating the Annotation type (that require user defined ctor)
 */
template<class Annotation>
struct tracing_annotation_context_t
{
	typedef Annotation annotation_type;
	typedef typename annotation_type::chain_type chain_type;

	enum { size = sizeof(annotation_type) };

	/*
	 * intentionally leave ctor and detor undefined to make class POD.
	 * this matters because global initialzers are always problematic.
	 */
#ifdef UFX_THIS_SYMBOL_IS_NEVER_DEFINED_PERIOD
	tracing_annotation_context_t();
	~tracing_annotation_context_t();
#endif

	annotation_type* self; 
	byte_t bytes[size];

	void init()
	{
		UF_ASSERT(!this->self);
		this->self = new (bytes) annotation_type();
	}

	void fini()
	{
		this->self->~annotation_type();
		this->self = 0; // for safe.
	}

	bool good() const { return 0 != this->self; }

	annotation_type* operator->() { return self; }
	const annotation_type* operator->() const { return self; }

	chain_type* chain() const { return good() ? &(self->chain()) : 0; }
};

UNFACT_NAMESPACE_EXTRAS_END

#endif//UNFACT_EXTRAS_TRACING_CHAIN_HPP

/* -*-
 Local Variables:
 mode: c++
 c-tab-always-indent: t
 c-indent-level: 2
 c-basic-offset: 2
 tab-width: 2
 End:
 -*- */
