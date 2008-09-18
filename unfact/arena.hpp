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

#ifndef UNFACT_ARENA_HPP
#define UNFACT_ARENA_HPP

#include <unfact/memory.hpp>
#include <unfact/algorithm.hpp>
#include <unfact/concurrent.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * "arena" is a fixed size allocator, intend to be used to implement collections.
 * the arena implementation assumes single-threaded access, so user should guarantee mutual access.
 *
 * arena allocates "pages" from source allocator. page is a large size of memory block,
 * from which it allocates multiple items. page size should be aligned to page/block size of 
 * underlying allocator, for efficiency reasons.
 *
 * deallocated items are called "slots", slots are cached inside arena and reused in
 * subsequent allocation. so we skip global allocation, 
 * that is generally slow - requres complex algorithm and care multi-threaded use.
 *
 * arena deallocates containing pages at its destruction, regardless there are used items.
 * users are strongly encouraged that deallocate all items before arena destruction.
 * (we alert when we find such "item leakage")
 *
 * theading model:
 * 
 * TODO: care concurrent access
 */
template<class Concurrent>
class basic_arena_t
{
public:
	typedef Concurrent concurrent_type;
	typedef typename concurrent_type::spin_lock_type lock_type;
	typedef basic_arena_t self_type;

	enum { page_alignment = 4 };

	enum option_e {
		option_none          = 0x0001,
		option_fill_efnoise = 0x0001,
		option_default = option_fill_efnoise,
		options,
	};

  struct chain_t
  {
		chain_t* m_next;
  };

  struct stat_t
  {
		stat_t() : m_npages(0), m_nslots(0) {}
		size_t m_npages;
		size_t m_nslots;
  };

  enum { chain_size = sizeof(chain_t) };

  basic_arena_t(allocator_t* allocator, size_t item_size, size_t page_size=DEFAULT_PAGE_SIZE, 
								size_t align=sizeof(void*), option_e opts=option_default)
		: m_allocator(allocator), 
			m_item_size(roundup_to_p2(max_of(item_size, to_size(chain_size)), align)),
			m_page_size(max_of(m_item_size*2, page_size)),
			m_size(0), m_options(opts),
			m_page_tail(0), m_slot_tail(0), m_npacked(0)
  {
		UF_ASSERT(sizeof(chain_t) <= m_item_size);
		UF_ASSERT(m_item_size*2 <= m_page_size);
  }

	/* dtor requires single mutual access! */
  ~basic_arena_t()
  {
		if (0 < m_size) { UF_ERROR(("found possible leak (%d items) in basic_arena_t", m_size)); }

		for (chain_t* ch = m_page_tail; 0 != ch;) {
			chain_t* todie = ch;
			ch = ch->m_next;
			m_allocator->deallocate(reinterpret_cast<byte_t*>(todie));
		}
  }

	template<class Synchronized>
  void exchange(basic_arena_t& other, const Synchronized&)
  {
		if (this == &other)  { return; } // ESSENTIAL: deadlock in such case

		lock_scope_t<self_type, Synchronized> lo(&other);
		lock_scope_t<self_type, Synchronized> lt(this);
		exchange_bytes(this, &other);
  }

  void exchange(basic_arena_t& other) {	exchange(other, synchronized_t()); }

  byte_t* allocate()
  {
		byte_t* ret = allocate_from_slot();
		
		if (!ret) {
			ret = allocate_from_page();
		}

		return ret;
  }

	template<class Synchronized>
  void deallocate(byte_t* ptr, const Synchronized&)
  {
		if (m_options & option_fill_efnoise) {
			memset(ptr, 0xef, m_item_size);
		}

		chain_t* chain = reinterpret_cast<chain_t*>(ptr);

		lock_scope_t<self_type, Synchronized> l(this);
		chain->m_next = m_slot_tail;
		m_slot_tail = chain;
		m_size--;
		m_stat.m_nslots++;
  }

  void deallocate(byte_t* ptr) { return deallocate(ptr, synchronized_t()); }

	/*
	 * these values are immutable for each instance, so safely nolock.
	 */
  size_t item_size() const { return m_item_size; }
  size_t page_size() const { return m_page_size; }
  allocator_t* allocator() const { return m_allocator; }

	template<class Synchronized>
  size_t size(const Synchronized&) const { return lock_scope_t<const self_type, Synchronized>(this)->m_size; }
  size_t size() const { return size(synchronized_t()); }

	void acquire() const { m_lock.acquire(); }
	void release() const { m_lock.release(); }

public: // expose implementation detail for testing and inspection

	template<class Synchronized>
	bool has_slots(const Synchronized&) const { return 0 != lock_scope_t<const self_type, Synchronized>(this)->m_slot_tail; }
	bool has_slots() const { return has_slots(synchronized_t()); }
	template<class Synchronized>
  size_t npages(const Synchronized&) const { return lock_scope_t<const self_type, Synchronized>(this)->m_stat.m_npages; }
  size_t npages() const { return npages(synchronized_t()); }
	template<class Synchronized>
  size_t nslots(const Synchronized&) const { return lock_scope_t<const self_type, Synchronized>(this)->m_stat.m_nslots; }
  size_t nslots() const { return nslots(synchronized_t()); }
	template<class Synchronized>
  size_t npacked(const Synchronized&) const { return lock_scope_t<const self_type, Synchronized>(this)->m_npacked; }
  size_t npacked() const { return npacked(synchronized_t()); }

private:
  basic_arena_t(const basic_arena_t&);
  const basic_arena_t& operator=(const basic_arena_t&);

	template<class Synchronized>
  byte_t* allocate_from_slot(const Synchronized&)
  {
		lock_scope_t<self_type, Synchronized> l(this);

		if (!m_slot_tail) {
			return 0;
		}

		chain_t* ret = m_slot_tail;
		m_slot_tail = ret->m_next;
		m_stat.m_nslots--;
		m_size++;

		return reinterpret_cast<byte_t*>(ret);
  }

  byte_t* allocate_from_slot() { return allocate_from_slot(synchronized_t()); }

	template<class Synchronized>
	byte_t* allocate_from_tail_page(const Synchronized&)
	{
		lock_scope_t<self_type, Synchronized> l(this);

		if (!room_available(unsynchronized_t())) { // unsynchronized here because we are already locked.
			return 0;
		}

		m_size++;
		return reinterpret_cast<byte_t*>(m_page_tail) + m_item_size*(m_npacked++);
	}

	byte_t* allocate_from_tail_page() {	return allocate_from_tail_page(synchronized_t());	}

	template<class Synchronized>
  byte_t* allocate_from_page(const Synchronized& sync)
  {
		byte_t* ret = 0;

		while (!ret) {
			ret = allocate_from_tail_page(sync);
			if (!ret) {
				byte_t* page = m_allocator->allocate(m_page_size);
				UF_ASSERT(address_aligned(page, page_alignment));
				UF_ALERT_AND_RETURN_UNLESS(page, 0, "can't allocate memory for new page!");
				bool ok = reserve_page_with(page, sync);
				if (!ok) {
					/* reservation is already done by another thread; so memory here is not used.  */
					m_allocator->deallocate(page);
				}
			}
		}

		return ret;
  }

  byte_t* allocate_from_page() { return allocate_from_page(synchronized_t()); }

	/*
	 * construct new page over memory pointed by 'ptr' parameter.
	 * we allocate 'ptr' memory outside this method to eliminate possible deadlock.
	 *
	 * @return True if the reservation succeeded, false if foreign thread have done the reservation.
	 *         Note that reservation itsself is never fail and done by someone. 
	 */
	template<class Synchronized>
	bool reserve_page_with(byte_t* ptr, const Synchronized&)
	{
		lock_scope_t<self_type, Synchronized> l(this);

		if (room_available(unsynchronized_t())) { // unsynchronized because we are already locked.
			/* new page is alerady reserved by another thread */
			return false; 
		}

		chain_t* tail = reinterpret_cast<chain_t*>(ptr);
		tail->m_next = m_page_tail;
		m_page_tail = tail;
		m_stat.m_npages++;
		m_npacked = 1; /* 1 for page chain */

		return true;
	}

	bool reserve_page_with(byte_t* ptr)	{	return reserve_page_with(ptr, synchronized_t()); }

	template<class Synchronized>
	bool room_available(const Synchronized&) const
	{
		lock_scope_t<const self_type, Synchronized> l(this);
		return m_page_tail && ((m_npacked + 1)*m_item_size <= m_page_size);
	}

private:
	mutable lock_type m_lock;
  allocator_t* m_allocator;
  size_t m_item_size;
  size_t m_page_size;
  size_t m_size;
	option_e m_options;
  chain_t* m_page_tail;
  chain_t* m_slot_tail;
  size_t   m_npacked;

  stat_t m_stat;
};

typedef basic_arena_t<null_concurrent_t> unconcurrent_arena_t;
typedef basic_arena_t<default_concurrent_t> arena_t; 

UNFACT_NAMESPACE_END

#endif//UNFACT_ARENA_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
