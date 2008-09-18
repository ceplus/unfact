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

#ifndef UNFACT_ALGORITHM_HPP
#define UNFACT_ALGORITHM_HPP

#include <unfact/base.hpp>
#include <unfact/concurrent.hpp>
#include <string.h>

UNFACT_NAMESPACE_BEGIN

/*
 * swap byte content. this is useful to swap non-publicly-copyable object.
 * but use carefully and you should know what this means!
 *
 * @param T should POD or equivalent
 */
template<class T>
inline void exchange_bytes(T* src, T* dst)
{
  if (src != dst) {
		byte_t buf[sizeof(T)];
		memcpy(buf, src, sizeof(T));
		memcpy(src, dst, sizeof(T));
		memcpy(dst, buf, sizeof(T));
  }
}

// std::swap equivalent: use different name to avoid ambiguity
template<class T>
inline void exchange(T& x, T& y) 
{
  T tmp = x;
  x = y;
  y = tmp;
}

template<class Iterator, class T, class Synchronized>
inline void fill(Iterator first, Iterator last, const T& val)
{
	for(Iterator i=first; i!=last; /* */) {
		lock_scope_t<Iterator, Synchronized> l(&i);
		*i = val;
		++i;
	}
}

template<class Iterator, class T>
inline void fill(Iterator first, Iterator last, const T& val)
{
	fill<Iterator, T, synchronized_t>(first, last, val);
}

template<class Iterator>
inline Iterator next(Iterator iter)
{
	++iter;
	return iter;
}

UNFACT_NAMESPACE_END

#endif//UNFACT_ALGORITHM_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width:2
	 End:
	 -*- */
