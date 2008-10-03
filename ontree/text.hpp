
#ifndef ONTREE_TEXT_HPP
#define ONTREE_TEXT_HPP

#include <ontree/base.hpp>
#include <ontree/strings.hpp>
#include <cstring>
#include <vector>

ONT_NAMESPACE_BEGIN

class text_t
{
public:
  text_t() : m_data(0), m_capacity(0) {}
  text_t(char* data, size_t capacity) 
	: m_data(data), m_capacity(capacity) {}
  const char* c_str() const { return m_data; }
  char* data() { return m_data; }
  size_t capacity() const { return m_capacity; }
  size_t size() const { return strlen(m_data); }
private:
  char* m_data;
  size_t m_capacity;
};

inline bool operator==(const text_t& x, const text_t& y) { return 0 == strcmp(x.c_str(), y.c_str()); }

inline bool operator==(const text_t& x, const char* yc) { return 0 == strcmp(x.c_str(), yc); }
inline bool operator==(const char* xc, const text_t& y) { return 0 == strcmp(xc, y.c_str()); }
inline bool operator!=(const text_t& x, const text_t& y) { return !(x == y); }
inline bool operator!=(const text_t& x, const char* yc) { return !(x == yc); }
inline bool operator!=(const char* xc, const text_t& y) { return !(xc == y); }

inline bool operator==(const text_t& x, const const_range_t& yc)
{
  size_t len = x.size();
  return ((len == yc.size()) && (0 == strncmp(x.c_str(), yc.head(), len)));
}

inline bool operator==(const const_range_t& xc, const text_t& y) { return y == xc; }
inline bool operator!=(const text_t& x, const const_range_t& yc) { return !(x == yc); }
inline bool operator!=(const const_range_t& xc, const text_t& y) { return !(xc == y); }



ONT_NAMESPACE_END

#endif//ONTREE_TEXT_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
