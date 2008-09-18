
#ifndef UNFACT_TEST_INSTANCE_COUNTED_HPP
#define UNFACT_TEST_INSTANCE_COUNTED_HPP

/*
 * A class that counts own instance count. made for testing memory leakage.
 *
 * Note that you should instansiate template with your own Tag class for each test
 * to make separate instance counter variables.
 */
template<class Tag>
struct instance_counted_t
{
  instance_counted_t(int i=0) : m_id(i) { instances()++; } // intentionally implicit
  instance_counted_t(const instance_counted_t& other) : m_id(other.m_id) { instances()++; }
  ~instance_counted_t() { instances()--; }
  int m_id;
  static int& instances() { static int s_instances = 0; return s_instances; }
};

template<class Tag>
inline bool operator<(const instance_counted_t<Tag>& x, const instance_counted_t<Tag>& y)
{
  return x.m_id < y.m_id;
}

#endif//UNFACT_TEST_INSTANCE_COUNTED_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
