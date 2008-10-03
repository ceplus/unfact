
#ifndef ONTREE_BASE_HPP
#define ONTREE_BASE_HPP

#include <cassert>

#define ONT_NAMESPACE_BEGIN namespace ontree {
#define ONT_NAMESPACE_END   }

#define ONT_NOT_IMPLEMENTED_YET() assert(!"not implemented yet!")
#define ONT_SHOULD_NOT_BE_REACHED() assert(!"shoult not be reached!")

ONT_NAMESPACE_BEGIN

enum error_e
{
  error_ok = 0,
  error_need_buffer,
  error_ill_formed,
  error_too_deep,
  error_not_found,
  error_unexpected,
  errors
};

inline bool is_ok(error_e err) { return err == error_ok; }

typedef unsigned char byte_t;
typedef float real_t;
class none_t {};

/*
 * copied from unfact 
 */
template <class T> inline T min_of(const T& x, const T& y) { return x < y ? x : y; } 
template <class T> inline T max_of(const T& x, const T& y) { return x < y ? y : x; } 

/*
 * roundup 'x' to modulo of 'mod'. we assume that 'mod' is power of 2
 */
inline size_t
roundup_to_p2(size_t x, size_t mod)
{
  size_t mask = (mod-1);
  return ((x&~mask) + ((x&mask) ? mod : 0));
}

ONT_NAMESPACE_END

#endif//ONTREE_BASE_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
