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

#ifndef UNFACT_PLATFORM_FILENAME_HPP
#define UNFACT_PLATFORM_FILENAME_HPP

#include <unfact/base.hpp>
#include <unfact/meta.hpp>
#include <unfact/string_ops.hpp>
#include <unfact/platform.hpp>

#ifdef UNFACT_PLATFORM_WINDOWS
# define UNFACT_FILENAME_SEPARATOR '\\'
#elif defined UNFACT_PLATFORM_EXTERNAL
/* you should provide your own */
#else
# define UNFACT_FILENAME_SEPARATOR '/'
#endif

UNFACT_NAMESPACE_BEGIN

inline const char* strip_directory(const char* filename)
{
  size_t len = string_ops_t<char>::count(filename);
  size_t i   = 0;
  for (i=0; i<len; ++i) {
	if (filename[len-i-1] == UNFACT_FILENAME_SEPARATOR) {
	  break;
	}
  }

  return filename + (len-i);
}

UNFACT_NAMESPACE_END

#endif//UNFACT_PLATFORM_FILENAME_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
