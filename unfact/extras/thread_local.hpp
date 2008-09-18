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

#ifndef UNFACT_EXTRAS_THREAD_LOCAL_HPP
#define UNFACT_EXTRAS_THREAD_LOCAL_HPP

#include <unfact/base.hpp>
#include <unfact/extras/base.hpp>
#include <unfact/concurrent.hpp>

UNFACT_NAMESPACE_EXTRAS_BEGIN

#ifdef UNFACT_PLATFORM_WINDOWS
template<size_t StorageID>
struct default_thread_local_t { typedef windows_thread_local_t<StorageID> type; };
#elif defined(UNFACT_PLATFORM_LINUX)
template<size_t StorageID>
struct default_thread_local_t { typedef posix_thread_local_t<StorageID> type; };
#elif defined(UNFACT_PLATFORM_EXTERNAL)
/* you should define default_thread_local_t somewhere */
#else
# error "unknown platform"
#endif

enum thread_local_id {
	thead_local_id_heap_tracing_annotation = 0,
	thead_local_id_tick_tracing_annotation,
	thead_local_ids
};

UNFACT_NAMESPACE_EXTRAS_END

#endif//UNFACT_EXTRAS_THREAD_LOCAL_HPP

/* -*-
 Local Variables:
 mode: c++
 c-tab-always-indent: t
 c-indent-level: 2
 c-basic-offset: 2
 tab-width: 2
 End:
 -*- */
