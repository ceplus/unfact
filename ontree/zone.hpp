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

#ifndef ONTREE_ZONE_HPP
#define ONTREE_ZONE_HPP

#include <ontree/base.hpp>
#include <cstring> // for memcpy()

ONT_NAMESPACE_BEGIN

class zone_t
{
public:
  enum {
	default_page_size = 1024,
	word_size = sizeof(byte_t*)
  };

  struct page_t {	page_t* m_next; };

  zone_t(size_t page_size=default_page_size)
	: m_page_size(roundup_to_p2(page_size, word_size)), m_head(0), m_ptr(0), m_end(0)
  {
	assert(sizeof(page_t) < m_page_size);
  }

  ~zone_t()
  {
	page_t* p = m_head;
	while (p) {
	  page_t* here = p;
	  p = p->m_next;
	  delete [] reinterpret_cast<byte_t*>(here);
	}
  }

  byte_t* allocate(size_t size)
  {
	size_t toalloc = roundup_to_p2(size, word_size); // word aligned

	if (m_end < m_ptr + toalloc) {
	  make_room(toalloc);
	  assert(m_ptr + toalloc <= m_end);
	}

	byte_t* ret = m_ptr;
	m_ptr += toalloc;

	return ret;
  }

  size_t page_size() const { return m_page_size; }

  void swap(zone_t& that)
  {
	// XXX: use unfact::exchange_bytes()
	if (this != &that) {
	  byte_t* dst = reinterpret_cast<byte_t*>(this);
	  byte_t* src = reinterpret_cast<byte_t*>(&that);
	  byte_t buf[sizeof(zone_t)];
	  memcpy(buf, src, sizeof(zone_t));
	  memcpy(src, dst, sizeof(zone_t));
	  memcpy(dst, buf, sizeof(zone_t));
	}
  }

private:
  zone_t(const zone_t&);
  const zone_t& operator=(const zone_t&);

  void make_room(size_t at_least)
  {
	size_t toalloc = max_of(m_page_size, sizeof(page_t) + at_least);
	page_t* next = reinterpret_cast<page_t*>(new byte_t[toalloc]);
	next->m_next = m_head;
	m_head = next;
	m_ptr  = reinterpret_cast<byte_t*>(next) + sizeof(page_t);
	m_end  = reinterpret_cast<byte_t*>(next) + toalloc;
  }

private:
  size_t  m_page_size;
  page_t* m_head;
  byte_t* m_ptr;
  byte_t* m_end;
};

ONT_NAMESPACE_END

#endif//ONTREE_ZONE_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
