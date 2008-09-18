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

#ifndef UNFACT_TYPES_HPP
#define UNFACT_TYPES_HPP

#include <stddef.h>

#ifdef _MSC_VER
# define UNFACT_PLATFORM_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__)
# define UNFACT_PLATFORM_LINUX
#elif defined(UNFACT_HAS_EXTERNAL_PLATFORM) // should be defined at <unfact_user_extension.hpp>
# define UNFACT_PLATFORM_EXTERNAL 
#else
# error "unrecogniezed platform!"
#endif

#define UNFACT_NAMESPACE_BEGIN namespace unfact {
#define UNFACT_NAMESPACE_END   }

UNFACT_NAMESPACE_BEGIN

typedef unsigned char byte_t;
typedef size_t size_t;

UNFACT_NAMESPACE_END

#endif//UNFACT_TYPES_HPP

/* -*-
 Local Variables:
 mode: c++
 c-tab-always-indent: t
 c-indent-level: 2
 c-basic-offset: 2
 tab-width: 2
 End:
 -*- */
