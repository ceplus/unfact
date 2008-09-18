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

#ifndef UNFACT_POSIX_PLATFORM_CONCURRENT_HPP
#define UNFACT_POSIX_PLATFORM_CONCURRENT_HPP

#include <unfact/base.hpp>
#include <unfact/platform/concurrent.hpp>
#include <pthread.h>
#include <atomic_ops.h>

UNFACT_NAMESPACE_BEGIN

/*
 * 
 */
struct ao_atomic_ops_t
{
  typedef AO_t value_type;

  static bool compare_and_swap(volatile value_type* addr, value_type oldval, value_type newval)
  {
		return 0 != AO_compare_and_swap_full(addr, oldval, newval);
  }

  static void barrier()
  {
		AO_nop_full();
  }

  /*
   * yielding with backoff: 
   * logic is cloned from boost/detail/spinlock_w32.hpp and slightly modied
   */
  static void yield_nth(size_t n)
  {
    if (n <  8) {
			return;
		} else if (n < 32) {
			struct timespec tosleep;
			tosleep.tv_sec = 0;
			tosleep.tv_nsec = 0;
			nanosleep(&tosleep, 0);
			return;
		} else {
			struct timespec tosleep;
			tosleep.tv_sec = 0;
			tosleep.tv_nsec = 1000;
			nanosleep(&tosleep, 0);
			return;
		}
  }
};

template<>
inline ao_atomic_ops_t::value_type
atomic_value_cast<ao_atomic_ops_t::value_type, int>(int x)
{
	return static_cast<ao_atomic_ops_t::value_type>(x);
}


template<size_t StorageID>
class posix_thread_local_t
{
public:
  typedef byte_t* value_type;
	
	posix_thread_local_t()
	{
		int err = pthread_key_create(&m_key, NULL);
		UF_ALERT_AND_RETURN_VOID_UNLESS(0 == err, "cannot allocate TLS key!");
	}

	~posix_thread_local_t()
	{
		pthread_key_delete(m_key);
	}

	value_type get() const { return reinterpret_cast<value_type>(pthread_getspecific(m_key)); }
	void set(value_type value) { pthread_setspecific(m_key, value); }
	void clear() { pthread_setspecific(m_key, 0); }

private:
	pthread_key_t m_key;
};

template<>
struct concurrent_t<posix_platform_tag_t>
{
  typedef spin_lock_t<ao_atomic_ops_t> spin_lock_type;
  typedef rw_lock_t<ao_atomic_ops_t, spin_lock_type> rw_lock_type;
};

UNFACT_NAMESPACE_END

#endif//UNFACT_POSIX_PLATFORM_CONCURRENT_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
