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

#ifndef ONTREE_STRINGS_HPP
#define ONTREE_STRINGS_HPP

#include <ontree/base.hpp>
#include <cstring>
#include <string>

ONT_NAMESPACE_BEGIN

class const_range_t
{
public:
  const_range_t() : m_head(0), m_size(0) {}
  const_range_t(const char* h, size_t s) : m_head(h), m_size(s) {}
  explicit const_range_t(const char* s)
	: m_head(s), m_size(strlen(s)) {}
  const char* head() const { return m_head; }
  size_t size() const { return m_size; }
  bool empty() const { return 0 == m_size; }

  char at(size_t i) const
  {
	assert(i < m_size);
	return m_head[i];
  }

  void consume(size_t sz)
  {
	assert(sz <= m_size);
	m_head += sz;
	m_size -= sz;
  }

  const_range_t consumed(size_t sz) const
  {
	assert(sz <= m_size);
	return const_range_t(m_head + sz, m_size - sz);
  }

  bool operator==(const const_range_t& that) const
  {
	return 0 == strncmp(that.head(),
						this->head(), min_of(that.size(), this->size()));
  }

  bool operator!=(const const_range_t& that) const
  {
	return !(*this == that);
  }

  // just for convenience: never used inside ontree
  std::string str() const { return std::string(m_head, m_size); }

private:
  const char* m_head;
  size_t  m_size;
};

class range_t
{
public:
  range_t() : m_head(0), m_size(0) {}
  range_t(char* h, size_t s) : m_head(h), m_size(s) {}
  explicit range_t(char* s)
	: m_head(s), m_size(strlen(s)) {}

  char* head() const { return m_head; }
  size_t size() const { return m_size; }
  bool empty() const { return 0 == m_size; }

  char at(size_t i) const
  {
	assert(i < m_size);
	return m_head[i];
  }

  void consume(size_t sz)
  {
	assert(sz <= m_size);
	m_head += sz;
	m_size -= sz;
  }

  range_t consumed(size_t sz) const
  {
	assert(sz <= m_size);
	return range_t(m_head + sz, m_size - sz);
  }

  bool operator==(const range_t& that) const
  {
	return 0 == strncmp(that.head(),
						this->head(), min_of(that.size(), this->size()));
  }

  bool operator!=(const range_t& that) const
  {
	return !(*this == that);
  }

  range_t write(const char* str, size_t len) const
  {
	assert(len <= size());
	memcpy(m_head, str, len);
	return range_t(m_head + len, m_size-len);
  }

  range_t write(char c) const
  {
	assert(1 <= size());
	m_head[0] = c;
	return range_t(m_head + 1, m_size-1);
  }

  range_t term() const
  {
	return write('\0');
  }

  std::string str() const { return std::string(m_head, m_size); }

private:
  char* m_head;
  size_t  m_size;
};

/*
 * @return 0 for broken char
 */
inline size_t
utf8_char_size(const char ch)
{
  if (0xf0 == (0xf0 & ch)) {        // 0b11110000
	return 4;
  } else if (0xe0 == (0xe0 & ch)) { // 0b11100000
	return 3;
  } else if (0xc0 == (0xc0 & ch)) { // 0b11000000
	return 2;
  } else if (0x80 == (0x80 & ch)) { // 0b10000000
	return 0; // broken
  } else {
	return 1;
  }
}

inline size_t
char_digit(char c)
{
  if ('0' <= c && c <= '9') {
	return c - '0';
  } else if ('a' <= c && c <= 'f') {
	return 10 + (c - 'a');
  } else if ('A' <= c && c <= 'F') {
	return 10 + (c - 'A');
  } else {
	ONT_SHOULD_NOT_BE_REACHED();
	return 0;
  }
}

/*
 * @param digits four digit part 'nnnn' of escaped unicode character '\unnnn'
 */
inline void
unescape_utf8_digits(const char* digits, char* out, size_t* written_bytes)
{
  size_t uchar = (char_digit(digits[0])*16*16*16 +
				  char_digit(digits[1])*16*16 +
				  char_digit(digits[2])*16 +
				  char_digit(digits[3])*1    );
  if (uchar <= 0x00007F) { 
	out[0] = char(uchar); // 0xxxxxxx 
	*written_bytes = 1;
  } else if (uchar <= 0x0007FF) {
	out[0] = 0xc0 | char((uchar >>    6)       ); //  5 bit:110xxxxx
	out[1] = 0x80 | char((uchar        ) & 0x3f); //  6 bit:10xxxxxx
	*written_bytes = 2;
  } else if (uchar <= 0x00FFFF) {
	out[0] = 0xe0 | char((uchar >>   12)       ); //  4 bit:1110xxxx
	out[1] = 0x80 | char((uchar >>    6) & 0x3f); //  6 bit:10xxxxxx
	out[2] = 0x80 | char((uchar        ) & 0x3f); //  6 bit:10xxxxxx
	*written_bytes = 3;
  } else if (uchar <= 0x10FFFF) {
	out[0] = 0xe0 | char((uchar >>   18)       ); //  3 bit:11110xxx
	out[1] = 0x80 | char((uchar >>   12) & 0x3f); //  6 bit:10xxxxxx
	out[2] = 0x80 | char((uchar >>    6) & 0x3f); //  6 bit:10xxxxxx
	out[3] = 0x80 | char((uchar        ) & 0x3f); //  6 bit:10xxxxxx
	*written_bytes = 4;
  } else {
	*written_bytes = 0; // for safe
  }
}

inline error_e
unescape(const const_range_t& from, char* out, size_t out_len, size_t* written_bytes)
{
  assert(0 < out_len);
  char* end = out + out_len - 1;
  char* p = out;
  size_t i = 0;
  while (i < from.size()) {
	if (p == end) {
	  return error_need_buffer;
	}

	char ch = from.at(i);
	if ('\\' != ch) { 
	  *p = ch;
	  ++p;
	  ++i;
	} else {
	  ++i;
	  char next = from.at(i);

	  switch (next) {
	  case '"':
	  case '\\':
	  case '/':
		*p = next;
		++p; ++i;
		break;
	  case 'b':
		*p = '\b';
		++p; ++i;
		break;
	  case 'f':
		*p = '\f';
		++p; ++i;
		break;
	  case 'n':
		*p = '\n';
		++p; ++i;
		break;
	  case 'r':
		*p = '\r';
		++p; ++i;
		break;
	  case 't':
		*p = '\t';
		++p; ++i;
		break;
		break;
	  case 'u': {
		if (from.size() - i < (1+4)) { // 1+4 == strlen("uxxxx")
		  return error_ill_formed;
		} else if (end - p < 4) {
		  return error_need_buffer;
		} else {
		  size_t char_bytes = 0;
		  unescape_utf8_digits(from.head() + (i+1), p, &char_bytes);
		  if (0 == char_bytes) {
			return error_ill_formed;
		  }

		  p += char_bytes;
		  i += 5;
		}
	  }	break;
	  default:
		return error_ill_formed;
	  }
	}
  }
		
  *(p++) = '\0';
  *written_bytes = p - out;

  return error_ok;
}

inline size_t
count_escape_overhead(const char* str, size_t len)
{
  size_t count = 0;

//          char = unescaped /
//                 escape (
//                     %x22 /          ; "    quotation mark  U+0022
//                     %x5C /          ; \    reverse solidus U+005C
//                     %x2F /          ; /    solidus         U+002F
//                     %x62 /          ; b    backspace       U+0008
//                     %x66 /          ; f    form feed       U+000C
//                     %x6E /          ; n    line feed       U+000A
//                     %x72 /          ; r    carriage return U+000D
//                     %x74 /          ; t    tab             U+0009
//                     %x75 4HEXDIG )  ; uXXXX                U+XXXX
//          escape = %x5C              ; \

  /*
   * we does not escape multibyte utf-8 characters for Japanese string debugability.
   */
  for (size_t i=0; i<len; /* */) {
	char c = str[i];
	if ('"'  == c ||
		'\\' == c ||
		'/'  == c ||
		'\b' == c ||
		'\f' == c ||
		'\n' == c ||
		'\r' == c ||
		'\t' == c) {
	  count += 1;
	}
	i += utf8_char_size(c);
  }
		
  return count;
}

inline size_t
count_escape_overhead(const char* str) { return count_escape_overhead(str, strlen(str)); }

inline error_e
escape(const const_range_t& from, char* out, size_t out_len, size_t* written_bytes)
{
  assert(0 < out_len);
  char* end = out + out_len - 1;
  char* p = out;
  size_t i = 0;
  while (i < from.size()) {
	char ch = from.at(i);
	size_t chsize = 1;
	size_t left = size_t(end - p);
	switch (ch) {
	case '"':
	case '\\':
	case '/':
	  if (left < 2) { return error_need_buffer; }
	  *(p++) = '\\';
	  *(p++) = ch;
	  break;
	case '\b':
	  if (left < 2) { return error_need_buffer; }
	  *(p++) = '\\';
	  *(p++) = 'b';
	  break;
	case '\f':
	  if (left < 2) { return error_need_buffer; }
	  *(p++) = '\\';
	  *(p++) = 'f';
	  break;
	case '\n':
	  if (left < 2) { return error_need_buffer; }
	  *(p++) = '\\';
	  *(p++) = 'n';
	  break;
	case '\r':
	  if (left < 2) { return error_need_buffer; }
	  *(p++) = '\\';
	  *(p++) = 'r';
	  break;
	case '\t':
	  if (left < 2) { return error_need_buffer; }
	  *(p++) = '\\';
	  *(p++) = 't';
	  break; 
	default:
	  chsize = utf8_char_size(ch);
	  if (left < chsize) {
		return error_need_buffer;
	  }

	  switch (chsize) {
	  case 0:
		return error_ill_formed;
	  case 1:
		*(p++) = ch;
		break;
	  default:
		/*
		 * currently we doesn't escape multibyte utf8 chars.
		 */
		if (from.size() < i+chsize) {
		  return error_ill_formed;
		}

		for (size_t j=0; j<chsize; ++j) {
		  *(p++)= from.at(i+j);
		}
		break;
	  }
	  break;
	}

	i += chsize;
  }
		
  *(p++) = '\0';
  *written_bytes = p - out;

  return error_ok;
}

ONT_NAMESPACE_END

#endif//ONTREE_STRINGS_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
