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

#ifndef UNFACT_TICK_TRACER_HPP
#define UNFACT_TICK_TRACER_HPP

#include <unfact/tree_tracer.hpp>
#include <unfact/keyed_value.hpp>
#include <unfact/string_ops.hpp>
#include <unfact/sticky.hpp>
#include <unfact/tick_ops.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * @todo doc
 * @todo add maximum
 * @todo add alerting
 *
 * @param StickyTrace impelemtation fo concept StickyTrace. see sticky.hpp for more detail.
 */
template<class StickyTrace, class Concurrent=null_concurrent_t>
class sticky_tracer_t
{
public:

  typedef StickyTrace trace_type;
	typedef typename trace_type::value_type value_type;
	typedef Concurrent concurrent_type;
	typedef sticky_tracer_t self_type;
  typedef tree_tracer_t<trace_type, concurrent_type> tracer_type;
  typedef typename tracer_type::key_type trace_key_type;
  typedef typename tracer_type::value_type trace_value_type;
  typedef typename tracer_type::iterator trace_iterator;
  typedef typename tracer_type::ticket_type ticket_type;

  sticky_tracer_t(allocator_t* allocator, 
								 size_t tracing_page_size=DEFAULT_PAGE_SIZE)
		: m_tracer(allocator, tracing_page_size)
  {}

  const tracer_type& tracer() const { return m_tracer; }

  void trace(ticket_type here, value_type value)
  {
		scalar_lock_scope_t<trace_iterator, synchronized_t> l(m_tracer.to_iterator(here));
		m_tracer.at(here).trace(value);
  }

  ticket_type root() const { return m_tracer.root(); }
  ticket_type parent(ticket_type here) const { return m_tracer.parent(here); }
  ticket_type push(ticket_type ticket, const trace_key_type& key) { return m_tracer.push(ticket, key); }
  ticket_type pop(ticket_type ticket) { return m_tracer.pop(ticket); }
  const trace_key_type& name_of(ticket_type ticket) const { return m_tracer.name_of(ticket); }
  const trace_value_type& at(ticket_type ticket) const { return m_tracer.at(ticket); }
  /* NOTE: we does not provide mutable at(). the data structure is read-only for outsiders. */
	void fill(value_type tofill=value_type(0)) { m_tracer.fill(trace_type(tofill, 0)); }
	void fill(ticket_type here, value_type tofill=value_type(0)) { m_tracer.fill(here, trace_type(tofill, 0)); }

	void acquire() const { m_tracer.acquire(); }
	void release() const { m_tracer.release(); }

private:
  tracer_type m_tracer;
};

template<class Tracer, class Duration=duration_blink_tag_t>
class tick_scope_t
{
public:
	typedef Tracer tracer_type;
	typedef typename tracer_type::ticket_type ticket_type;
	typedef tick_ops_t<default_platform_tag_t, Duration> ops_type;
	typedef typename ops_type::value_type tick_value_type;

	/*
	 * copyable value object: useful for FindKey for associative container
	 */
	class initializer_t
	{
	public:
		initializer_t(tracer_type* tracer, ticket_type here)
			: m_tracer(tracer), m_here(here) {}

		tracer_type* tracer() const { return m_tracer; }
		ticket_type  here() const { return m_here; }
	private:
		tracer_type* m_tracer;
		ticket_type  m_here;
	};

	tick_scope_t(tracer_type* tracer, ticket_type here):
		m_tracer(tracer), m_here(here), m_start(tracer ? ops_type::tick() : tick_value_type())
	{}

	tick_scope_t(const initializer_t& init):
		m_tracer(init.tracer()), m_here(init.here()), m_start(m_tracer ? ops_type::tick() : tick_value_type())
	{}

	~tick_scope_t()
	{
		if (m_tracer) {
			tick_value_type end = ops_type::tick();
			m_tracer->trace(m_here, ops_type::to_milliseconds(ops_type::distance(m_start, end)));
		}
	}

	ticket_type here() const { return m_here; }
private:
	tick_scope_t(const tick_scope_t&);
	const tick_scope_t& operator=(const tick_scope_t&);

	tracer_type* m_tracer;
	ticket_type  m_here;
	tick_value_type m_start;
};

template<class Tracer, class Duration>
class less_t< tick_scope_t<Tracer, Duration> >
{
public:
	typedef tick_scope_t<Tracer, Duration> key_type;
	typedef typename key_type::ticket_type ticket_type;
  bool operator()(const key_type& x, const key_type& y) const { return x.here() < y.here(); }
  bool operator()(const key_type& x, ticket_type yk) const { return x.here() < yk; }
  bool operator()(ticket_type xk, const key_type& y) const { return xk < y.here(); }
};

/*
 * typical (and currently only possible) instance of heap_tracer_t.
 */
typedef sticky_accumulation_t<float> tick_accumulation_t;
typedef sticky_tracer_t<tick_accumulation_t, default_concurrent_t> accumulative_tick_tracer_t;
typedef tick_scope_t<accumulative_tick_tracer_t> accumulative_tick_scope_t;
typedef flat_tracing_formatter_t<accumulative_tick_tracer_t::tracer_type> accumulative_tick_tracing_formatter_t;

// TODO: formatter here

UNFACT_NAMESPACE_END

#endif//UNFACT_TICK_TRACER_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width:2
	 End:
	 -*- */
