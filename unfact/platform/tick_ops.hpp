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

#ifndef UNFACT_PLATFORM_TICK_OPS_HPP
#define UNFACT_PLATFORM_TICK_OPS_HPP

#include <unfact/base.hpp>
#include <unfact/meta.hpp>
#include <unfact/platform.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * tick_ops_t is collection of tick related method/types
 */
class duration_blink_tag_t {}; // suited for a few millisecs. expect no/few context switch.
class duration_while_tag_t {}; // suited for a hundres millisecs or seconds, promissing context swicth.

template<class Platform, class Duration=duration_blink_tag_t>
struct tick_ops_t
{
	/*
	 * should be overriden
	 */
  typedef none_t value_type; 
  typedef none_t delta_type;
	/*
	 * should be defined
	 */
	static value_type tick();
	static delta_type distance(value_type from, value_type to);
	static float      to_milliseconds(delta_type x);
};


UNFACT_NAMESPACE_END

#endif//UNFACT_PLATFORM_TICK_OPS_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
