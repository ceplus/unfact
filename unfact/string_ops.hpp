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

#ifndef UNFACT_STRING_OPS_HPP
#define UNFACT_STRING_OPS_HPP

#include <unfact/base.hpp>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h> // sprintf_s

/*
 * XXX: kick these workaround out to platform speicific file
 */
#ifdef UNFACT_PLATFORM_WINDOWS
# define snprintf sprintf_s
#endif

UNFACT_NAMESPACE_BEGIN

/*
 * string operation interface:
 * implementations are given for each platform
 * we need this indirection because of lack of 
 * common wide-char routines around the various platforms.
 */
template<class T>
struct string_ops_t
{
public:
  static int    compare(const T* x, const T* y);
  static size_t count(const T* str);
  static void   copy(T* dst, size_t dst_size, const T* str);
};

template<>
struct string_ops_t<char>
{
public:
  static int    compare(const char* x, const char* y) { return strcmp(x, y); }
  static size_t count(const char* str) { return strlen(str); }
  static void   copy(char* dst, size_t dst_size, const char* str)
  {
#ifdef UNFACT_PLATFORM_WINDOWS
		strcpy_s(dst, dst_size, str);
#else
		strncpy(dst, str, dst_size);
#endif
  }

};

template<>
struct string_ops_t<wchar_t>
{
public:
  static int    compare(const wchar_t* x, const wchar_t* y) { return wcscmp(x, y); }
  static size_t count(const wchar_t* str) { return wcslen(str); }

  static void   copy(wchar_t* dst, size_t dst_size, const wchar_t* str)
  {
#ifdef UNFACT_PLATFORM_WINDOWS
		wcscpy_s(dst, dst_size, str);
#else
		wcsncpy(dst, str, dst_size);
#endif
  }

};

UNFACT_NAMESPACE_END

#endif//UNFACT_STRING_OPS_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
