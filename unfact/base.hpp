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

#ifndef UNFACT_BASE_HPP
#define UNFACT_BASE_HPP

#ifdef  UNFACT_HAS_USER_EXTENSION
# define UNFACT_DEFINING_USER_EXTENSION_BASE
# include <unfact_user_extension.hpp>
# undef  UNFACT_DEFINING_USER_EXTENSION_BASE
#endif//UNFACT_HAS_USER_EXTENSION

#include <unfact/types.hpp>
#include <unfact/log.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * define std::min and std::max alternative; win32 SDK has own min/value macro definition,
 * which is incompatible against STL semantics.
 */
template <class T> inline T min_of(const T& x, const T& y) { return x < y ? x : y; } 
template <class T> inline T max_of(const T& x, const T& y) { return x < y ? y : x; } 
template <class T> inline size_t to_size(const T& x) { return static_cast<size_t>(x); }
template <class T> inline int    to_i(const T& x) { return static_cast<int>(x); }

/* gcc workaround: they canno find  to_size<>() above */
inline size_t to_size(int x) { return static_cast<size_t>(x); }

/*
 * roundup 'x' to modulo of 'mod'. we assume that 'mod' is power of 2
 */
inline size_t
roundup_to_p2(size_t x, size_t mod)
{
  size_t mask = (mod-1);
  return ((x&~mask) + ((x&mask) ? mod : 0));
}

/*
 * return true if address 'x' is aligned 'size' byte.
 *
 * @param size should be power of 2
 */
template<class T>
inline bool
address_aligned(const T* x, size_t size)
{
	return 0 == (reinterpret_cast<size_t>(x) & (size-1));
}

UNFACT_NAMESPACE_END


#endif//UNFACT_BASE_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width:2
*/
