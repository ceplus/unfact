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

#ifndef UNFACT_POSIX_PLATFORM_TICK_HPP
#define UNFACT_POSIX_PLATFORM_TICK_HPP

#include <unfact/base.hpp>
#include <unfact/platform/tick_ops.hpp>
#include <time.h>

UNFACT_NAMESPACE_BEGIN


template<class Duration>
struct tick_ops_t<posix_platform_tag_t, Duration>
{
	typedef clock_t value_type;
	typedef size_t delta_type;
	
	static value_type tick() 
	{ 
		return ::clock();
	}

	static delta_type distance(value_type from, value_type to)
	{
		return delta_type(to - from);
	}

	static float to_milliseconds(delta_type x)
	{
		return float(x*1000)/float(CLOCKS_PER_SEC);
	}

};



UNFACT_NAMESPACE_END

#endif//UNFACT_POSIX_PLATFORM_TICK_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
