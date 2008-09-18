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

#ifndef UNFACT_PLATFORM_HPP
#define UNFACT_PLATFORM_HPP

#include <unfact/base.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * We provide platform specific routines as template classes and functions.
 * Following platform tags are used to specialize such templates:
 *
 * - windows_platform_tag_t for MS windows platform
 * - posix_platform_tag_t for POSIX compatible platform
 * - null_platform_tag_t to disable/fake platform specific routines.
 *   Disabling lock/unlock on single-threaded code is one of our usecases.
 * 
 * Users who have defined UNFACT_PLATFORM_EXTERNAL should define default_platform_tag_t
 * to identify their own platform, and also should provide specialized templates 
 * for each platform specific routines.
 */
class windows_platform_tag_t { int dummy; };
class posix_platform_tag_t { int dummy; };
class null_platform_tag_t { int dummy; };
class external_platform_tag_t { int dummy; };

#if   defined UNFACT_PLATFORM_WINDOWS
typedef windows_platform_tag_t default_platform_tag_t;
#elif defined UNFACT_PLATFORM_LINUX
typedef posix_platform_tag_t default_platform_tag_t;
#elif defined UNFACT_PLATFORM_EXTERNAL
typedef external_platform_tag_t default_platform_tag_t;
#else
# error "unknown platform"
#endif

UNFACT_NAMESPACE_END

#endif//UNFACT_PLATFORM_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   tab-width: 2
   End:
   -*- */
