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

#ifndef UNFACT_WINDOWS_PLATFORM_TICK_HPP
#define UNFACT_WINDOWS_PLATFORM_TICK_HPP

#include <unfact/base.hpp>
#include <unfact/platform/tick_ops.hpp>
#include <windows.h>

UNFACT_NAMESPACE_BEGIN


template<>
struct tick_ops_t<windows_platform_tag_t, duration_blink_tag_t>
{
	typedef LARGE_INTEGER value_type;
	typedef size_t delta_type;
	
	static value_type tick() 
	{ 
		LARGE_INTEGER result;
		result.QuadPart = 0;
		BOOL ok = QueryPerformanceCounter(&result);
		UF_ALERT_AND_RETURN_UNLESS(ok, result, "QueryPerformanceCounter() can no be used!");
		return result;
	}

	static delta_type distance(value_type from, value_type to)
	{
		return delta_type(to.QuadPart - from.QuadPart);
	}

	static float to_milliseconds(delta_type x)
	{
		LARGE_INTEGER freq;
		BOOL ok = QueryPerformanceFrequency(&freq);
		UF_ALERT_AND_RETURN_UNLESS(ok, float(x), "QueryPerformanceFrequency() can no be used!");
		return (static_cast<float>(x)/static_cast<float>(freq.QuadPart))*1000.0f;
	}

};

template<>
struct tick_ops_t<windows_platform_tag_t, duration_while_tag_t>
{
	typedef DWORD value_type;
	typedef size_t delta_type;
	
	static value_type tick() 
	{ 
		return GetTickCount();
	}

	static delta_type distance(value_type from, value_type to)
	{
		return to - from;
	}

	static float to_milliseconds(delta_type x)
	{
		return static_cast<float>(x);
	}

};


UNFACT_NAMESPACE_END

#endif//UNFACT_WINDOWS_PLATFORM_TICK_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
