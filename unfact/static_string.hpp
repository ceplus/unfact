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

#ifndef UNFACT_STATIC_STRING_HPP
#define UNFACT_STATIC_STRING_HPP

#include <unfact/base.hpp>
#include <unfact/string_ops.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * static_string_t is stack-allocated, fixed-length string implementation.
 * it can hold string shorter than limit length.
 * the length is given as template parameter. 
 *
 * internal string representation is null-terminated character array (as usual C.)
 * we choose null-termination instead of tracking the length
 * because of easy debugging and simplicity.
 * it is more debugger friendly than unterminated representations
 *
 * TODO: implements append() and friends
 */

template<class Char, size_t Capacity = (sizeof(void*)*4 -1) >
class basic_static_string_t
{
public:
  enum { capacity = Capacity };
  typedef Char value_type;
  typedef string_ops_t<value_type> ops_type;

  const value_type* c_str() const { return m_body; }
  size_t size() const { return ops_type::count(m_body); }

  value_type  operator[](size_t i) const 
  {
		/* although we should guard against size() for security, it'll take cost. so we comporomize to such */
		UF_ASSERT(0 <= i && i < capacity);
		return m_body[i];
  }

  value_type& operator[](size_t i)
  {
		/* although we should guard against size() for security, it'll take cost. so we comporomize to such */
		UF_ASSERT(i < capacity);
		return m_body[i];
  }

  basic_static_string_t() { m_body[0] = static_cast<value_type>(0); }

  /* intentionally implicit */
  basic_static_string_t(const value_type* str)
  {
		size_t len = min_of(size_t(capacity), length(str));
		memcpy(m_body, str, len*sizeof(value_type));
		m_body[len] = static_cast<value_type>(0);
  }

  bool operator==(const basic_static_string_t& that) const
  {
		return 0 == ops_type::compare(this->m_body, that.m_body);
  }

  bool operator!=(const basic_static_string_t& that) const
  {
		return !(*this == that);
  }

  bool operator<(const basic_static_string_t& that) const
  {
		return ops_type::compare(this->m_body, that.m_body) < 0;
  }

  /*
   * generic string routines:
   */
  static size_t length(const value_type* str)
  {
		int n=0;
		while (str[n]) { n++; }
		return n;
  }

private:
  value_type m_body[capacity+1];
};

UNFACT_NAMESPACE_END

#endif//UNFACT_STATIC_STRING_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
