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

#ifndef UNFACT_EXTRAS_BASE_HPP
#define UNFACT_EXTRAS_BASE_HPP

#include <unfact/base.hpp>
#include <unfact/memory.hpp>
#include <unfact/platform.hpp>

#define UNFACT_NAMESPACE_EXTRAS_BEGIN namespace unfact { namespace extras {
#define UNFACT_NAMESPACE_EXTRAS_END   } }

#ifdef  UNFACT_HAS_USER_EXTENSION
# define UNFACT_EXTRAS_DEFINING_USER_EXTENSION_BASE
# include <unfact_user_extension.hpp>
# undef  UNFACT_EXTRAS_DEFINING_USER_EXTENSION_BASE
#endif//UNFACT_HAS_USER_EXTENSION

UNFACT_NAMESPACE_EXTRAS_BEGIN

#ifndef UNFACT_EXTRAS_HAS_USER_BACKDOOR_ALLOCATOR
typedef stdlib_allocator_t backdoor_allocator_t;
#endif

UNFACT_NAMESPACE_EXTRAS_END

#endif//UNFACT_EXTRAS_BASE_HPP

/* -*-
 Local Variables:
 mode: c++
 c-tab-always-indent: t
 c-indent-level: 2
 c-basic-offset: 2
 tab-width: 2
 End:
 -*- */
