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

#ifndef UNFACT_WINDOWS_PLATFORM_CONCURRENT_HPP
#define UNFACT_WINDOWS_PLATFORM_CONCURRENT_HPP

#include <unfact/base.hpp>
#include <unfact/platform/concurrent.hpp>
#include <windows.h>

// see http://msdn.microsoft.com/en-us/library/f20w0x5e(VS.80).aspx
extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)

UNFACT_NAMESPACE_BEGIN

/*
 * ao_atomic_ops_t implements imaginary AtomicOps concept that provides CPU depenent atomic operation.
 * Such atomic operations are mainly used to implement lightweight synchronization primitive like spin locks.
 *
 * although yield_nth() is not an atomic operation, we give it here for convenience.
 */
struct windows_atomic_ops_t
{
  typedef byte_t* value_type;

  static bool compare_and_swap(volatile value_type* addr, value_type oldval, value_type newval)
  {
		return 
			(oldval == ::InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(addr), newval, oldval));
  }

  static void barrier()
  {
		::_ReadWriteBarrier();
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
			::Sleep(0);
			return;
		} else {
			::Sleep(1);
			return;
		}
  }
};

template<size_t StorageID>
class windows_thread_local_t
{
public:
  typedef byte_t* value_type;
	
	windows_thread_local_t()
		: m_index(TlsAlloc())
	{
		UF_ALERT_AND_RETURN_VOID_UNLESS(TLS_OUT_OF_INDEXES != m_index, "cannot allocate TLS!");
	}

	~windows_thread_local_t()
	{
		if (m_index != TLS_OUT_OF_INDEXES) {
			BOOL ok = TlsFree(m_index);
			UF_ALERT_AND_RETURN_VOID_UNLESS(ok, "TlsFree() failed!");
		}
	}

	value_type get() const { return reinterpret_cast<value_type>(TlsGetValue(m_index)); }
	void set(value_type value) { TlsSetValue(m_index, reinterpret_cast<LPVOID>(value)); }
	void clear() { TlsSetValue(m_index, 0); }

private:
	DWORD m_index;
};

template<>
struct concurrent_t<windows_platform_tag_t>
{
  typedef spin_lock_t<windows_atomic_ops_t> spin_lock_type;
  typedef rw_lock_t<windows_atomic_ops_t, spin_lock_type> rw_lock_type;
};

UNFACT_NAMESPACE_END

#endif//UNFACT_WINDOWS_PLATFORM_CONCURRENT_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
