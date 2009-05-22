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

#ifndef ONTREE_OBUFSTREAM_HPP
#define ONTREE_OBUFSTREAM_HPP

#include <ontree/base.hpp>
#include <ostream>

ONT_NAMESPACE_BEGIN

class bufstreambuf_t : public std::basic_streambuf<char>
{
public:
  /* copied from std spec 27.5 */
  typedef char char_type;
  typedef std::char_traits<char> traits_type;
  typedef traits_type::int_type int_type;
  typedef traits_type::pos_type pos_type;
  typedef traits_type::off_type off_type;
public:
  explicit bufstreambuf_t(char_type* buf, size_t buflen)
	: m_buf(buf), m_buflen(buflen), m_written(0), m_overflowed(false)
  {}

  bufstreambuf_t* self() { return this; }

  const char_type* buffer() const { return m_buf; }
  char_type* buffer() { return m_buf; }
  size_t buffer_size() const { return m_buflen; }
  size_t written() const { return m_written; }
  bool overflowed() const { return m_overflowed; }

  // 27.5.2.4.2 Buffer management and positioning:
  virtual int sync()
  {
	return 0; // we never fail. spec says -1 on failure.
  }

  // 27.5.2.4.5 Put area:

  virtual std::streamsize xsputn(const char_type* s, std::streamsize n)
  {
	size_t left = m_buflen - m_written;
	if (size_t(n) < left) { 
	  size_t toput = min_of(size_t(n), left);
	  memcpy(m_buf + m_written, s, toput);
	  m_written += toput;
	  return std::streamsize(toput);
	} else {
	  m_overflowed = true;
	  return 0;
	}
  }

  virtual int_type overflow (int_type c = traits_type::eof())
  {
	if (traits_type::eq_int_type(c,traits_type::eof())) {
	  return traits_type::not_eof(c);
	} else {
	  char p = static_cast<char>(c);
	  xsputn(&p, 1);
	  return c;
	}
  }

  void terminate_cstr()
  {
	if (m_written < m_buflen) {
	  m_buf[m_written++] = '\0';
	} else {
	  m_overflowed = true;
	}
  }

private:
  char_type* m_buf;
  size_t     m_buflen;
  size_t     m_written;
  bool m_overflowed;
};

class obufstream_t : 
  bufstreambuf_t, public std::ostream // use inheritance in CRTP manner
{
public:
  obufstream_t(char* buf, size_t len)
	: bufstreambuf_t(buf, len), std::ostream(self())
  {}

  void terminate_cstr() {  bufstreambuf_t::terminate_cstr(); }
  const char* buffer() const { return bufstreambuf_t::buffer(); }	
  size_t buffer_size() const { return bufstreambuf_t::buffer_size(); }
  size_t written() const { return bufstreambuf_t::written(); }
  bool overflowed() const { return bufstreambuf_t::overflowed(); }
};

ONT_NAMESPACE_END

#endif//ONTREE_OBUFSTREAM_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
