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

#ifndef UNFACT_HEAP_TRACER_HPP
#define UNFACT_HEAP_TRACER_HPP

#include <unfact/tree_tracer.hpp>
#include <unfact/keyed_value.hpp>
#include <unfact/string_ops.hpp>
#include <unfact/delta.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * heap tracer is designed to trace malloc()-free() invocation sequences.
 * heap_tracer_t provides map from allocated heap to its allocation context.
 * collected data is kept inside tree_tracer_t<DeltaTrace> and is avaialbe at trace() accessor.
 *
 * @todo add maximum
 * @todo add alerting

 * @param DeltaTrace impelemtation fo concept DeltaTrace. see delta.hpp for more detail.
 */
template<class DeltaTrace, class Concurrent=null_concurrent_t>
class heap_tracer_t
{
public:

  typedef DeltaTrace trace_type;
	typedef Concurrent concurrent_type;
	typedef heap_tracer_t self_type;
  typedef tree_tracer_t<trace_type, concurrent_type> tracer_type;
  typedef typename tracer_type::key_type trace_key_type;
  typedef typename tracer_type::value_type trace_value_type;
  typedef typename tracer_type::iterator trace_iterator;
  typedef typename tracer_type::ticket_type ticket_type;

  class heap_node_t
  {
  public:
		heap_node_t(ticket_type t=0, size_t s=0)
			: m_ticket(t), m_size(s) {}

		ticket_type ticket() const { return m_ticket; }
		size_t size() const { return m_size; }

  private:
		ticket_type m_ticket;
		size_t m_size;
  };

  typedef keyed_value_t<byte_t*, heap_node_t> heap_set_node_type;
  typedef tree_set_t<heap_set_node_type, less_t<heap_set_node_type>, concurrent_type> heap_set_type;
  typedef typename heap_set_type::const_iterator heap_iterator;

  heap_tracer_t(allocator_t* allocator, 
								size_t tracing_page_size=DEFAULT_PAGE_SIZE,
								size_t heap_page_size=DEFAULT_PAGE_SIZE)
		: m_tracer(allocator, tracing_page_size), 
			m_heaps(allocator, heap_page_size),
			m_size(0)
  {}

  const tracer_type& tracer() const { return m_tracer; }
  heap_iterator heap_begin() const { return m_heaps.begin(); }
  heap_iterator heap_end() const { return m_heaps.end(); }

	/*
	 * @return ptr
	 */
  void* trace_allocated(ticket_type here, byte_t* ptr, size_t size)
  {
		heap_iterator i	= m_heaps.insert(heap_set_node_type(ptr, heap_node_t(here, size)));
		UF_HONOR_OR_RETURN(i != m_heaps.end(), ptr);

		raise_size(size);

		scalar_lock_scope_t<typename tracer_type::iterator, synchronized_t> l(tracer_type::to_iterator(here));
		m_tracer.at(here).trace_raised(size);

		return ptr;
  }

  void trace_deallocated(byte_t* ptr)
  {
		/*
		 * It may appear t that find() following remove() has a race at first glance.
		 * But it is OK because we kow that 'ptr' is owned by single object
		 * and should never be deleted simultaneously.
		 */
		heap_iterator hi = m_heaps.find(ptr);
		UF_HONOR_OR_RETURN_VOID(hi != m_heaps.end());
		heap_node_t h = hi->value();
		m_heaps.remove(hi);

		fall_size(h.size());

		scalar_lock_scope_t<typename tracer_type::iterator, synchronized_t> l(tracer_type::to_iterator(h.ticket()));
		m_tracer.at(h.ticket()).trace_fallen(h.size());
  }

  size_t size() const { return m_size; }

  ticket_type root() const { return m_tracer.root(); }
  ticket_type parent(ticket_type here) const { return m_tracer.parent(here); }
  ticket_type push(ticket_type ticket, const trace_key_type& key) { return m_tracer.push(ticket, key); }
  ticket_type pop(ticket_type ticket) { return m_tracer.pop(ticket); }
  const trace_key_type& name_of(ticket_type ticket) const { return m_tracer.name_of(ticket); }
  const trace_value_type& at(ticket_type ticket) const { return m_tracer.at(ticket); }
  /* NOTE: we does not provide mutable at(). the data structure is read-only for outsiders. */

	void acquire() const { m_heaps.acquire(); }
	void release() const { m_heaps.release(); }

public: // implementation detail
	void raise_size(size_t sz) { lock_scope_t<self_type, synchronized_t> l(this); m_size += sz; }
	void fall_size(size_t sz) {  lock_scope_t<self_type, synchronized_t> l(this); m_size -= sz; }

private:
  tracer_type m_tracer;
  heap_set_type m_heaps;
  size_t m_size;
};

/*
 * typical (and currently only possible) instance of heap_tracer_t.
 */
typedef delta_accumulation_t<size_t, int> heap_accumulation_t;
typedef heap_tracer_t<heap_accumulation_t, default_concurrent_t> accumulative_heap_tracer_t;
typedef accumulation_formatter_t<accumulative_heap_tracer_t::tracer_type> accumulative_heap_tracing_formatter_t;

UNFACT_NAMESPACE_END

#endif//UNFACT_HEAP_TRACER_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width:2
	 End:
	 -*- */
