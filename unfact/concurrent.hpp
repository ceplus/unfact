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

#ifndef UNFACT_CONCURRENT_HPP
#define UNFACT_CONCURRENT_HPP

#include <unfact/base.hpp>
#include <unfact/platform.hpp>

#ifdef  UNFACT_HAS_USER_EXTENSION
# define UNFACT_DEFINING_USER_EXTENSION_CONCURRENT
# include <unfact_user_extension.hpp>
# undef  UNFACT_DEFINING_USER_EXTENSION_CONCURRENT
#endif//UNFACT_HAS_USER_EXTENSION

/*
 * select concurrent_t instance.
 * see platform/concurrent.hpp for detailed description.
 */

#if defined UNFACT_PLATFORM_WINDOWS
# include <unfact/platform/windows/concurrent.hpp>
#elif defined UNFACT_PLATFORM_LINUX
# include <unfact/platform/posix/concurrent.hpp>
#elif defined UNFACT_PLATFORM_EXTERNAL
/* you should provide your own */
#else 
# error "unknown platform"
#endif

# include <unfact/platform/null/concurrent.hpp>

/*
 * provide some typical instantiations
 */
UNFACT_NAMESPACE_BEGIN

typedef concurrent_t<default_platform_tag_t> default_concurrent_t;
typedef concurrent_t<null_platform_tag_t> null_concurrent_t;

UNFACT_NAMESPACE_END

#endif//UNFACT_CONCURRENT_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   tab-width:2
   End:
   -*- */
