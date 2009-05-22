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

#ifndef UNFACT_STICKY_HPP
#define UNFACT_STICKY_HPP

#include <unfact/tree_tracer.hpp>
#include <unfact/keyed_value.hpp>
#include <unfact/string_ops.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * TODO: doc
 */

/*
 * 
 * @param Modulo : interval of samples on which we clear the collected value to avoid overflow.
 *                 it is unnecessary when Value is floating point value
 */
template<class Value, size_t Modulo=16777216>
class sticky_accumulation_t
{
public:
	typedef Value value_type;
	enum { modulo_size = Modulo };

  sticky_accumulation_t() : m_total(0), m_samples(0) {}
  sticky_accumulation_t(value_type t, size_t s) : m_total(t), m_samples(s) {}

	value_type average() const { return 0 < m_samples ? m_total/static_cast<value_type>(m_samples) : 0; }
	value_type total() const { return m_total; }
	size_t samples() const { return m_samples; }

	void trace(value_type value)
	{
		size_t s = m_samples + 1;
		if (modulo_size <= s) {
			m_samples = 1;
			m_total = value;
		} else {
			m_samples = s;
			m_total += value;
		}
	}

	void clear(value_type toclear=value_type(0))
	{
		m_total = toclear;
		m_samples = 0;
	}

private:
	value_type m_total;
  size_t m_samples;
};

template<class Tracer>
class flat_tracing_formatter_t
{
public:
	typedef Tracer tracer_type;
	typedef typename tracer_type::value_type value_type;
	typedef typename tracer_type::ticket_type ticket_type;
  typedef typename tracer_type::iterator iterator_type;

	flat_tracing_formatter_t(const tracer_type* tracer, char* buf, size_t bufsize,
													 ticket_type root)
		: m_tracer(tracer),  m_buf(buf), m_bufsize(bufsize),
			m_here(tracer->begin_for(root)), m_end(tracer->end_for(root))
	{ format(); }

  bool atend() const { return m_here == m_end; }
  const char* c_str() const { return m_buf; }

	void increment()
	{
		++m_here;
		format();
	}
	
  void format()
  {
		if (atend()) {
			m_buf[0] = '\0';
		} else {
			value_type value = m_tracer->at(tracer_type::to_ticket(m_here));
			int printed = snprintf(m_buf, m_bufsize, "%8.1f (%6d times):", value.average(), value.samples());
			if (m_bufsize-1 <= static_cast<size_t>(printed)) {
				return; // filled
			}
	
			char* buf = m_buf + printed;
			size_t dummy = 0;
			m_tracer->format_name(m_here, buf, m_bufsize - printed, &dummy);
		}
	}
	
private:
  const tracer_type* m_tracer;
	char*  m_buf;
  size_t m_bufsize;
  iterator_type m_here;
  iterator_type m_end;

};
	
UNFACT_NAMESPACE_END

#endif//UNFACT_STICKY_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width:2
	 End:
	 -*- */
