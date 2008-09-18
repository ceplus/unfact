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

#ifndef UNFACT_DELTA_HPP
#define UNFACT_DELTA_HPP

#include <unfact/tree_tracer.hpp>
#include <unfact/keyed_value.hpp>
#include <unfact/string_ops.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * a collection of DeltaTrace concept implementations.
 * currently we only have delta_accumulation_t.
 *
 * DeltaTrace concept is designed to collect delta time-series data
 * like malloc()-free() invocations, that provide deltas of scalar values.
 */

/*
 * delta_accumulation_t is a implementation of DeltaTrace imaginary concept 
 * that sum up raise and fall of the given delta, for tracing final scala.
 */
template<class Delta, class Scalar=Delta>
class delta_accumulation_t
{
public:
	typedef Delta delta_type;
	typedef Scalar scalar_type;

  delta_accumulation_t() : m_raised(0), m_fallen(0), m_samples(0) {}

  scalar_type final() const { return static_cast<scalar_type>(m_raised) - static_cast<scalar_type>(m_fallen); }
  delta_type raised() const { return m_raised; }
  delta_type fallen() const { return m_fallen; }
  size_t samples() const { return m_samples; }

  void trace_raised(delta_type sz) { m_raised += sz; m_samples++; }
  void trace_fallen(delta_type sz) { m_fallen += sz; }

private:
  delta_type m_raised;
  delta_type m_fallen;
  size_t m_samples;
};

/*
 * sum up toal finals of accumulation_count sequence
 *
 * @todo think about thread safety
 */
template<class Tracer, class Synchronized>
inline size_t accumulation_count(const Tracer& tracer, typename Tracer::ticket_type t, const Synchronized&)
{
  typedef typename Tracer::iterator iter_type;

	lock_scope_t<const Tracer, Synchronized> l(&tracer);
  size_t count = 0;
  iter_type beg = tracer.begin_for(t, unsynchronized_t());
  iter_type end = tracer.end_for(t, unsynchronized_t());
  for(iter_type i=beg; i != end; ++i) {	count += i->value().final();  }
  return count;
}

template<class Tracer>
inline size_t accumulation_count(const Tracer& tracer, typename Tracer::ticket_type t)
{
	return accumulation_count(tracer, t, synchronized_t());
}

/*
 * format reporting text of delta_accumulation tree.
 *
 * thread-safety:
 * accumulation_formatter_t is NOT (and possibly never) thread-safe. 
 * You havefa  responsibility to lock the Tracer instance. 
 * thread-safety for accumulation_formatter_t is hard because the instance have
 * a iterator to format, that may be invalidated by any other thread.
 *
 * We know that IO ops including printing formatted text are typically slow and
 * synchronizations which containing such one have large performance penalty. 
 * So we should have non-blocking tree treversal in future version of unfact.
 *
 * TODO: add thread-safe format printing function (which may require a heavy lock.)
 *
 * @param Trace should be instance of accumulation_formatter_t Tracer.
 *        typically tree_tracer_t<accumulation_formatter_t>
 */
template<class Tracer>
class accumulation_formatter_t
{
public:
  typedef Tracer tracer_type;
  typedef typename tracer_type::ticket_type ticket_type;
  typedef typename tracer_type::iterator iterator_type;
	enum { max_height = 16 };

  accumulation_formatter_t(const tracer_type* tracer, char* buf, size_t bufsize,
													 ticket_type root)
		: m_tracer(tracer), m_buf(buf), m_bufsize(bufsize),
			m_count(0), m_height(0),
			m_here(tracer->begin_for(root)), m_end(tracer->end_for(root))
  {
		memset(m_counts, 0, max_height*sizeof(size_t));
		UF_HONOR_OR_RETURN_VOID(1 <= m_bufsize); // we need at least '\0'
		m_buf[0] = '\n';
		countup_and_format();
  }

  bool atend() const { return m_here == m_end; }
  const char* c_str() const { return m_buf; }

  void increment_one()
  {
		++m_here;
		countup_and_format();
  }

	void increment()
	{
		do { 
			increment_one();
		} while (0 == m_counts[m_height] && !atend());
	}

public: // implementation detail

	void countup_and_format()
	{
		countup();
		format();
	}

	void countup()
	{
		if (0 < m_height) { m_counts[m_height-1] += m_counts[m_height];	}
		m_counts[m_height] = 0;

		if (!atend()) {
			size_t h = m_here.height();
			UF_ALERT_AND_RETURN_VOID_UNLESS(h < max_height, "trace tree is too heigh!");
			m_height = h;
			m_counts[m_height] += m_here->value().final();
		}
	}

  void format()
  {
		if (atend()) {
			m_buf[0] = '\0';
		} else {
			size_t count = m_counts[m_height];
			int printed = snprintf(m_buf, m_bufsize, "%9d:", count);
			if (m_bufsize-1 <= static_cast<size_t>(printed)) {
				return; // filled
			}
	
			char* buf = m_buf + printed;
			size_t dummy = 0;
			m_tracer->format_name(m_here, buf, m_bufsize - printed, &dummy);
		}
  }

public: // utiilty for test
  void format_name(ticket_type here, char* buf, size_t bufsize, size_t* written) const
  {
		m_tracer->format_name(m_tracer->to_iterator(here), buf, bufsize, written);
  }

private:
  const tracer_type* m_tracer;
  char*  m_buf;
  size_t m_bufsize;
  size_t m_count;
	size_t m_counts[max_height];
	size_t m_height;
  iterator_type m_here;
  iterator_type m_end;
};

UNFACT_NAMESPACE_END

#endif//UNFACT_DELTA_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width:2
	 End:
	 -*- */
