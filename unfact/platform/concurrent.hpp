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

#ifndef UNFACT_PLATFORM_CONCURRENT_HPP
#define UNFACT_PLATFORM_CONCURRENT_HPP

#include <unfact/base.hpp>
#include <unfact/meta.hpp>
#include <unfact/platform.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * almost similar to boost/detail/yield_k.hpp
 */
template<class Platform>
void yield_therad_k(size_t k);


#pragma warning(push)
#pragma warning(disable : 4312)
template<class To, class From>
inline To atomic_value_cast(From x) { return reinterpret_cast<To>(x); }
#pragma warning(pop)

template<class AtomicOps>
inline void advance(volatile typename AtomicOps::value_type* value, int delta)
{
  typename AtomicOps::value_type x;
  do {
		x = *value;
  } while (!AtomicOps::compare_and_swap(value, x, x+delta));
}

/*
 * spinlock implementation:
 * logic is cloned from boost/detail/spinlock_w32.hpp
 */
template<class AtomicOps>
class spin_lock_t
{
public:
  typedef AtomicOps ops_type;
  typedef typename ops_type::value_type value_type;

  spin_lock_t() : m_value(0) {}

  ~spin_lock_t()
  {
		ops_type::barrier();
		UF_HONOR_OR_RETURN_VOID(0 == m_value);
  }

  bool try_acquire() volatile {
		bool ok = ops_type::compare_and_swap(&m_value, 
																				 atomic_value_cast<value_type>(0), 
																				 atomic_value_cast<value_type>(1));
		ops_type::barrier(); // i don't know why this fence is required...
		return ok;
  }

  void acquire() volatile
  {
		size_t n = 0;
		while (!try_acquire()) {
			ops_type::yield_nth(n++);
		}
  }

  void release() volatile
  {
		ops_type::barrier();
		m_value = 0;
  }

private:
  spin_lock_t(const spin_lock_t&);
  spin_lock_t& operator=(const spin_lock_t&);

private:
  volatile value_type m_value;
};

template<class AtomicOps, class Lock>
class rw_lock_t
{
public:
  typedef AtomicOps ops_type;
  typedef typename ops_type::value_type value_type;
  typedef Lock lock_type;
  
  rw_lock_t() : m_nreaders(0) {}

  ~rw_lock_t()
  {
		UF_HONOR_OR_RETURN_VOID(0 == m_nreaders);
  }

  void read_acquire() volatile
  {
		m_writing.acquire();
		advance<ops_type>(&m_nreaders,  1);
		m_writing.release();
  }

  void read_release() volatile
  {
		advance<ops_type>(&m_nreaders, -1);
  }

  void write_acquire() volatile
  {
		m_writing.acquire();
		wait_until_readers_gone();
  }

  void write_release() volatile
  {
		m_writing.release();
  }

private:
  rw_lock_t(const rw_lock_t&);
  rw_lock_t& operator=(const rw_lock_t&);

public: // impl detail
  void wait_until_readers_gone() volatile
  {
		size_t n = 0;
		ops_type::barrier();
		while (atomic_value_cast<value_type>(0) < m_nreaders) {
			ops_type::yield_nth(n++);
			ops_type::barrier();
		}
  }

private:
  volatile value_type m_nreaders;
  volatile lock_type m_writing;
};

/*
 * concurrent_t is a facade/traits to platform specific concurrent operations and datatypes
 * like synchronziation primitives and thread related APIs.
 * Because each operations and datatype is defined separetedly, 
 * so you can use such objects indivisually. 
 * concurrent_t just bring together them.
 */
template<class Platform>
struct concurrent_t
{
  typedef none_t spin_lock_t; // should be overriden
  typedef none_t rw_lock_t;   // should be overriden
};

/*
 * wrap acquire/release free floating function to 
 * specialize them
 */
template<class Lock>
inline void acquire_lock(const Lock* lock) { lock->acquire(); }
template<class Lock>
inline void release_lock(const Lock* lock) { lock->release(); }

class synchronized_t
{
public:
	template<class Lock>
	static void enter(const Lock& lock) { acquire_lock(lock); }
	template<class Lock>
	static void leave(const Lock& lock) { release_lock(lock); }
};

class unsynchronized_t
{
public:
	template<class Lock>
	static void enter(const Lock& lock) {}
	template<class Lock>
	static void leave(const Lock& lock) {}
};

template<class Lock, class Synchronized=synchronized_t>
class lock_scope_t
{
public:
	typedef Lock lock_type;
	typedef Synchronized sync_type;
	lock_scope_t(lock_type* l) : m_lock(l) { sync_type::enter(m_lock); }
	~lock_scope_t() { sync_type::leave(m_lock); }

	lock_type* operator->() const { return m_lock; }
	lock_type& operator*() const { return *m_lock; }

private:
	lock_scope_t(const lock_scope_t&);
	const lock_scope_t& operator=(const lock_scope_t&);
private:
	lock_type* m_lock;
};

template<class Scalar, class Synchronized>
class scalar_lock_scope_t
{
public:
	typedef Scalar scalar_type;
	typedef Synchronized sync_type;
	explicit scalar_lock_scope_t(scalar_type lock) : m_lock(lock) { sync_type::enter(&m_lock); }
	~scalar_lock_scope_t() { sync_type::leave(&m_lock); }
private:
	scalar_type m_lock;
};

UNFACT_NAMESPACE_END

#endif//UNFACT_PLATFORM_CONCURRENT_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
