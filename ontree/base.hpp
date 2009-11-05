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

#ifndef ONTREE_BASE_HPP
#define ONTREE_BASE_HPP

#include <cassert>
#include <cstddef>

#define ONT_NAMESPACE_BEGIN namespace ontree {
#define ONT_NAMESPACE_END   }

#define ONT_NOT_IMPLEMENTED_YET() assert(!"not implemented yet!")
#define ONT_SHOULD_NOT_BE_REACHED() assert(!"shoult not be reached!")

ONT_NAMESPACE_BEGIN

enum error_e
{
  error_ok = 0,
  error_need_buffer,
  error_ill_formed,
  error_too_deep,
  error_not_found,
  error_unexpected,
  errors
};

inline bool is_ok(error_e err) { return err == error_ok; }

typedef unsigned char byte_t;
class none_t {};

#ifdef ONT_CONFIG_USE_DOUBLE_REAL
typedef double real_t;
#else
typedef float  real_t;
#endif

/*
 * copied from unfact 
 */
template <class T> inline T min_of(const T& x, const T& y) { return x < y ? x : y; } 
template <class T> inline T max_of(const T& x, const T& y) { return x < y ? y : x; } 

/*
 * roundup 'x' to modulo of 'mod'. we assume that 'mod' is power of 2
 */
inline size_t
roundup_to_p2(size_t x, size_t mod)
{
  size_t mask = (mod-1);
  return ((x&~mask) + ((x&mask) ? mod : 0));
}

ONT_NAMESPACE_END

#endif//ONTREE_BASE_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
