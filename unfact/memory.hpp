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

#ifndef UNFACT_MEMORY_HPP
#define UNFACT_MEMORY_HPP

#include <unfact/base.hpp>
#include <stdlib.h>

UNFACT_NAMESPACE_BEGIN

enum {
	DEFAULT_PAGE_SIZE =
#ifdef UNFACT_HAS_DEFAULT_PAGE_SIZE
	UNFACT_DEFAULT_PAGE_SIZE
#else
	1024
#endif
};

class allocator_t
{
public:
  virtual ~allocator_t() {}
  virtual byte_t* allocate(size_t size) = 0;
  virtual void    deallocate(byte_t* ptr) = 0;
};

class stdlib_allocator_t : public allocator_t
{
public:
  virtual byte_t* allocate(size_t size) { return reinterpret_cast<byte_t*>(malloc(size)); }
  virtual void    deallocate(byte_t* ptr) { free(ptr); }
};

UNFACT_NAMESPACE_END

#endif//UNFACT_MEMORY_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   tab-width:2
   End:
   -*- */
