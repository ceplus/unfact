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

#ifndef UNFACT_EXTRAS_WINDOWS_LOG_HPP
#define UNFACT_EXTRAS_WINDOWS_LOG_HPP

#include <unfact/base.hpp>
#include <unfact/log.hpp>
#include <unfact/platform/filename.hpp>
#include <stdlib.h>
#include <windows.h>

/*
 * This file is intend to be included from unfact_user_extension.hpp to define log_printf().
 * Because depending on extras header files will complicate the dependency, 
 * windows_log_printf() is under unfact namespace, although the file itsself is under extras package.
 */
UNFACT_NAMESPACE_BEGIN

inline void windows_log_printf(const char* file, size_t line, log_level_e lv, 
															 const char* format, va_list ap)
{
	char buf [256];
  int len0 = _snprintf_s(buf, 256, _TRUNCATE, "%s:%d:UNFACT[%s] ", 
												 strip_directory(file), line, log_level_str(lv));
  _vsnprintf_s(buf + len0, 256-len0, _TRUNCATE, format, ap);
  OutputDebugStringA(buf);
  OutputDebugStringA("\n");

  if (unfact::log_level_fatal == lv) {
		OutputDebugStringA("abort!\n");
		abort();
  }
}

UNFACT_NAMESPACE_END

#endif//UNFACT_EXTRAS_WINDOWS_LOG_HPP

/* -*-
 Local Variables:
 mode: c++
 c-tab-always-indent: t
 c-indent-level: 2
 c-basic-offset: 2
 tab-width: 2
 End:
 -*- */
