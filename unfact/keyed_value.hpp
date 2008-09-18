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

#ifndef UNFACT_KEYED_VALUE_HPP
#define UNFACT_KEYED_VALUE_HPP

#include <unfact/base.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * alternative to std::pair
 */
template<class Key, class Value>
class keyed_value_t
{
public:
  typedef Key   key_type;
  typedef Value value_type;

  keyed_value_t(const key_type& key=key_type(), const value_type& value=value_type())
		: m_key(key), m_value(value) { }

	template<class K, class V>
  keyed_value_t(const keyed_value_t<K, V>& other)
		: m_key(other.key()), m_value(other.value()) { }
	
  const key_type& key() const { return m_key; }
  const value_type& value() const { return m_value; }
  value_type& value() { return m_value; }
  void set_value(const value_type& v) { m_value = v; }

  bool operator==(const keyed_value_t& that) const { return m_key == that.m_key && m_value == that.m_value; }
  bool operator!=(const keyed_value_t& that) const { return !(*this == that); }

private:
  key_type m_key;
  value_type m_value;
};

template<class K, class V>
inline bool operator<(const keyed_value_t<K,V>& x, const keyed_value_t<K,V>& y)
{
  return x.key() < y.key();
}

template<class AK, class K, class V>
inline  bool operator<(const keyed_value_t<K,V>& x, const AK& yk)
{
  return x.key() < yk;
}

template<class AK, class K, class V>
inline  bool operator<(const AK& xk, const keyed_value_t<K,V>& y)
{
  return xk < y.key();
}

UNFACT_NAMESPACE_END

#endif//UNFACT_KEYED_VALUE_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
