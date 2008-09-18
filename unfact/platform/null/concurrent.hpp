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

#ifndef UNFACT_NULL_PLATFORM_CONCURRENT_HPP
#define UNFACT_NULL_PLATFORM_CONCURRENT_HPP

#include <unfact/base.hpp>
#include <unfact/platform/concurrent.hpp>

UNFACT_NAMESPACE_BEGIN

class null_lock_t
{
public:
	null_lock_t() {}

  void acquire() {}
	void release() {}

private:
	null_lock_t(const null_lock_t&);
	const null_lock_t& operator=(const null_lock_t&);
};

class null_rw_lock_t
{
public:
	null_rw_lock_t() {}
  void read_acquire() {}
	void read_release() {}
  void write_acquire() {}
	void write_release() {}

private:
	null_rw_lock_t(const null_rw_lock_t&);
	const null_rw_lock_t& operator=(const null_rw_lock_t&);
};

template<>
struct concurrent_t<null_platform_tag_t>
{
  typedef null_lock_t spin_lock_type;
  typedef null_rw_lock_t rw_lock_type;
};

UNFACT_NAMESPACE_END

#endif//UNFACT_NULL_PLATFORM_CONCURRENT_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width:2
	 End:
	 -*- */
