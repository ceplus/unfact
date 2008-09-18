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

#ifndef UNFACT_TREE_SET_HPP
#define UNFACT_TREE_SET_HPP

#include <unfact/base.hpp>
#include <unfact/arena.hpp>
#include <unfact/algorithm.hpp>
#include <unfact/red_black.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * tree_set_t is a std::set-like generic collection based on red_black_t.
 * API does not align STL style. 
 * it is not intentional, but jusr short of effort. we welcome your contribution ;-)
 *
 * for ordering criteria, see the comment for red_black_t. 
 *
 * WARNING:
 * tree_set_t returns non-const iterator for your convenience.
 * but such iterators give you the way to modify item on the container.
 * changing item can cause ORDER INCONSISTENCY for the set, so you should do it carefully.
 */

/*
 * tree_set_skeleton_t is a helper class to implement tree_set_t and its variant.
 * tree_set_skeleton_t has only root node, but does not have sharable elements,
 * including arena and comparator.
 *
 */
template<class Key, class Comparator=less_t<Key>, class Subclass=none_t, 
				 class Concurrent=null_concurrent_t>
class tree_set_skeleton_t
{
public:
	typedef Concurrent concurrent_type;
	typedef typename concurrent_type::spin_lock_type lock_type;
	typedef basic_arena_t<concurrent_type> arena_type;
  typedef tree_set_skeleton_t self_type;
  typedef Key key_type;
  typedef Comparator comparator_type;
  typedef Subclass subclass_type;
  typedef typename red_black_t<key_type, comparator_type, subclass_type>::self_type node_type;
  typedef typename node_type::removal_unbalance_t removal_unbalance_type;
  typedef red_black_dfs_iterator_t<key_type, comparator_type, subclass_type> dfs_iterator_type;
  typedef node_type item_type;

  class const_iterator_t : public red_black_iterator_base_t<const_iterator_t, 
																														key_type, comparator_type, subclass_type>
  {
  public:
		typedef red_black_iterator_base_t<const_iterator_t, key_type, comparator_type, subclass_type> base_type;
		typedef const typename node_type::key_type  value_type;
		typedef const typename node_type::key_type* pointer_type;

		explicit const_iterator_t(node_type* n=0) : base_type(n) {}
		value_type& operator*() const { return base_type::node()->key(); }
		pointer_type operator->() const { return &(base_type::node()->key()); }
  };

  class iterator_t : public red_black_iterator_base_t<iterator_t,
																											key_type, comparator_type, subclass_type>
  {
  public:
		typedef red_black_iterator_base_t<iterator_t, key_type, comparator_type, subclass_type> base_type;
		typedef typename node_type::key_type  value_type;
		typedef typename node_type::key_type* pointer_type;

		iterator_t() : base_type(0) {}
		explicit iterator_t(node_type* n) : base_type(n) {}

		value_type& operator*() const { return base_type::node()->key(); }
		pointer_type operator->() const { return &(base_type::node()->key()); }

		// down covnertion
		operator const_iterator_t() const { return const_iterator_t(base_type::node()); }
  };

	template<class Synchronized>
  iterator_t begin(const Synchronized& sync) { return iterator_t(begin_node(sync)); }
  iterator_t begin() { return begin(synchronized_t()); }
	template<class Synchronized>
  const_iterator_t begin(const Synchronized& sync) const { return const_iterator_t(begin_node(sync)); }
  const_iterator_t begin() const { return begin(synchronized_t()); }
	template<class Synchronized>
  iterator_t end(const Synchronized&) { return iterator_t(0); }
  iterator_t end() { return end(synchronized_t()); }
	template<class Synchronized>
  const_iterator_t end(const Synchronized&) const { return const_iterator_t(0); }
  const_iterator_t end() const { return end(synchronized_t()); }
	template<class Synchronized>
  dfs_iterator_type dfs_begin(const Synchronized& sync) const { return dfs_iterator_type(root(sync)); }
  dfs_iterator_type dfs_begin() const { return dfs_begin(synchronized_t()); }
	template<class Synchronized>
  dfs_iterator_type dfs_end(const Synchronized&) const { return dfs_iterator_type(0); }
  dfs_iterator_type dfs_end() const { return dfs_end(synchronized_t()); }

  template<class FindKey, class Synchronized>
  const_iterator_t find(const comparator_type& compare, const FindKey& k, const Synchronized& sync) const { return const_iterator_t(find_node(compare, k, sync)); }
  template<class FindKey>
  const_iterator_t find(const comparator_type& compare, const FindKey& k) const { return find(compare, k, synchronized_t()); }
  template<class FindKey, class Synchronized>
  iterator_t find(const comparator_type& compare, const FindKey& k, const Synchronized& sync) { return iterator_t(find_node(compare, k, sync)); }
  template<class FindKey>
  iterator_t find(const comparator_type& compare, const FindKey& k) { return find(compare, k, synchronized_t()); }

  /*
   * @return end() when there is already same key
   *         or iterator that points inserted key
   */
	template<class NewKey, class Synchronized>
  iterator_t insert(arena_type* arena, const comparator_type& compare, const NewKey& key, 
										const Synchronized& sync)
  {
		node_type* toins = new_node(arena, key);
		UF_ALERT_AND_RETURN_UNLESS(toins, iterator_t(0), "failed to allocate new node!");

		iterator_t ret = insert_node(compare, toins, sync);
		if (ret.atend()) {
			delete_node(arena, toins);
		}

		return ret;
  }

	template<class NewKey>
  iterator_t insert(arena_type* arena, const comparator_type& compare, const NewKey& key)
	{
		return insert(arena, compare, key, synchronized_t());
	}

  /*
   * remove node pointed by the iterator.
   * the iterator "i" goes invalid after removal.
   */
  template<class Iterator, class Synchronized> // Iterator should be iterator_t or const_iterator_t
  void remove(arena_type* arena, Iterator i, const Synchronized& sync)
  { 
		if (i.atend()) {
			return;
		}

		remove_node(i.node(), sync);
		delete_node(arena, i.node());
  }

  template<class Iterator>
  void remove(arena_type* arena, Iterator i)
	{
		remove(arena, i, synchronized_t());
	}

  /*
   * same as insert(), except this method returns existing iterator instead of end() iter
   * when specified key is already inserted. 
   */
	template<class NewKey, class Synchronized>
  iterator_t ensure(arena_type* arena, const comparator_type& compare, const NewKey& key, 
										const Synchronized& sync)
  {
		iterator_t found = end(sync);
		while (found.atend()) {
			iterator_t found = find(compare, key, sync);
			if (found.good()) {
				return found;
			}

			node_type* toins = new_node(arena, key);
			UF_ALERT_AND_RETURN_UNLESS(toins, iterator_t(0), "failed to allocate new node!");
			iterator_t ret = insert_node(compare, toins, sync);
			if (ret.atend()) {
				/*
				 * another thread have already inserted same node: 
				 * so we remove ours and retry find again.
				 */ 
				delete_node(arena, toins);
			}
		}

		return found;
  }

	template<class NewKey>
  iterator_t ensure(arena_type* arena, const comparator_type& compare, const NewKey& key)
	{
		return ensure(arena, compare, key, synchronized_t());
	}

  template<class Iterator>
  void insert(arena_type* arena, const comparator_type& compare, Iterator beg, Iterator end)
  {
		for (Iterator i=beg; i!=end; ++i) {
			iterator_t inserted = insert(arena, compare, *i);
			UF_HONOR_OR_RETURN_VOID(!inserted.atend());
		}
  }

	/*
	 * CAUTION:
	 * clear() semantics goes complicated on concurrent world.
	 * our policy following:
	 * - during clear() is called, no other threads enver call neither remove() nor clear().
	 *   because all existing iterators go invalid.
	 * - insert() is OK. iserted nodes are live after clear()
	 */
  void clear(arena_type* arena)
  {
		node_type* old_root = pop_root();
		if (!old_root) {
			return;
		}

		removal_unbalance_type unb;
		for (dfs_iterator_type i = dfs_iterator_type(old_root), end = dfs_iterator_type(0); i != end; /* */) {
			dfs_iterator_type toremove = i++;
			node_type::remove_unbalanced(old_root, toremove.node(), &unb);
			delete_node(arena, toremove.node());
		}
  }

	template<class Synchronized>
  bool invariant(const Synchronized&) const
	{
		lock_scope_t<const self_type, Synchronized> l(this);
		return (0 == m_root) || m_root->invariant();
	}

	bool invariant() const { return invariant(synchronized_t()); }
	
	template<class Synchronized>
  bool empty(const Synchronized&) const { return 0 == lock_scope_t<const self_type, synchronized_t>(this)->m_root; }
	bool empty() const { return empty(synchronized_t()); }

	template<class Synchronized>
  size_t count(const Synchronized&) const
	{
		lock_scope_t<const self_type, synchronized_t> l(this);
		return m_root ? m_root->size() : 0;
	}

	size_t count() const { return count(synchronized_t()); }

	template<class Synchronized>
	node_type* root(const Synchronized&) const { return lock_scope_t<const self_type, Synchronized>(this)->m_root; }
  node_type* root() const { return root(synchronized_t()); }

	void acquire() const { m_lock.acquire(); }
	void release() const { m_lock.release(); }

  tree_set_skeleton_t()	: m_root(0) {}

  ~tree_set_skeleton_t()
  {
		/*
		 * all nodes should be released before destruction.
		 * it is user's responsibility
		 */
		UF_ASSERT(!m_root);
  }

	template<class Synchronized>
	void exchange(tree_set_skeleton_t& that, const Synchronized&)
	{
		if (this == &that)  { return; } // ESSENTIAL: deadlock in such case
		lock_scope_t<self_type, Synchronized> lo(&that);
		lock_scope_t<self_type, Synchronized> lt(this);
		unfact::exchange(m_root, that.m_root);
	}
	
	void exchange(tree_set_skeleton_t& that) { exchange(synchronized_t()); }

public: // impelemntation detail

	template<class Synchronized>
  node_type* begin_node(const Synchronized&) const
	{
		lock_scope_t<const self_type, Synchronized> l(this);
		return m_root ? m_root->minimum() : 0;
	}

  node_type* begin_node() const { begin_node(synchronized_t()); }

	template<class Synchronized>
	node_type* pop_root(const Synchronized&)
	{
		lock_scope_t<const self_type, Synchronized> l(this);
		node_type* ret = m_root;
		m_root = 0;
		return ret;
	}

	node_type* pop_root()	{ return pop_root(synchronized_t()); }

	template<class Synchronized>
  iterator_t insert_node(const comparator_type& compare, node_type* node, const Synchronized&)
	{
		lock_scope_t<const self_type, Synchronized> l(this);

		if (!m_root) {
			node->set_black();
			m_root = node;
			return iterator_t(node);
		}

		bool ok = node_type::insert(m_root, node, &m_root, compare);
		if (!ok) {
			return end();
		}

		return iterator_t(node);
	}

  iterator_t insert_node(const comparator_type& compare, node_type* node)
	{
		return insert_node(compare, node, synchronized_t());
	}

	template<class Synchronized>
  void remove_node(node_type* node, const Synchronized&)
	{
		lock_scope_t<const self_type, Synchronized> l(this);
		/* NOTE:
		 * non-null root assumption is OK even in concurrent world. 
		 * Because we should have at least one node if we have node to remove. 
		 */
		UF_ASSERT(m_root); 
		node_type::remove(m_root, node, &m_root);
	}

	void remove_node(node_type* node) { remove_node(node, synchronized_t()); }

  template<class FindKey, class Synchronized>
  node_type* find_node(const comparator_type& compare, const FindKey& k, const Synchronized&) const
  {
		lock_scope_t<const self_type, Synchronized> l(this);
		return (m_root ? const_cast<node_type*>(m_root->find(k, compare)) : 0);
  }

  template<class FindKey>
  node_type* find_node(const comparator_type& compare, const FindKey& k) const
	{
		return find_node(compare, k, synchronized_t());
	}

  void delete_node(arena_type* arena, node_type* node)
  {
		node->~node_type();
		arena->deallocate(reinterpret_cast<byte_t*>(node));
  }

	template<class NewKey>
  node_type* new_node(arena_type* arena, const NewKey& key)
  {
		void* mem = arena->allocate();
		UF_ALERT_AND_RETURN_UNLESS(mem, 0, "faile to allocate memory for new node!");
		return new (mem) node_type(key);
  }

private:
  tree_set_skeleton_t(const tree_set_skeleton_t&);
  tree_set_skeleton_t& operator=(tree_set_skeleton_t&);
private:
	mutable lock_type  m_lock;
  node_type* m_root;
};

/*
 * tree_set_t is a self-contained container. 
 * actual API implementation is done by tree_set_skeleton_t, 
 * so tree_set_t is just a aggregation of its building blocks like arena and comparator
 */
template<class Key, class Comparator=less_t<Key>, class Concurrent=null_concurrent_t>
class tree_set_t
{
public:
  typedef tree_set_t self_type;
  typedef Key key_type;
  typedef Comparator comparator_type;
	typedef Concurrent concurrent_type;
	typedef basic_arena_t<concurrent_type> arena_type;
  typedef tree_set_skeleton_t<key_type, comparator_type, none_t, concurrent_type> skeleton_type;
  typedef typename skeleton_type::iterator_t iterator;  
  typedef typename skeleton_type::const_iterator_t const_iterator;  
	typedef typename skeleton_type::item_type item_type;

  iterator begin() { return m_skeleton.begin(); }
  const_iterator begin() const { return m_skeleton.begin(); }
  iterator end() { return m_skeleton.end(); }
  const_iterator end() const { return m_skeleton.end(); }

  template<class FindKey>
  const_iterator find(const FindKey& k) const { return m_skeleton.find(m_compare, k); }
  template<class FindKey>
  iterator find(const FindKey& k) { return m_skeleton.find(m_compare, k); }

  template<class FindKey>
  bool contains(const FindKey& k) const { return end() != find(k); } /* thread-safe: end() is stateless*/

  template<class Iterator>
  void remove(Iterator i) { m_skeleton.remove(&m_arena, i); }
	template<class NewKey>
  iterator insert(const NewKey& key) { return m_skeleton.insert(&m_arena, m_compare, key);  }
	template<class NewKey>
  iterator ensure(const NewKey& key) { return m_skeleton.ensure(&m_arena, m_compare, key);  }
  template<class Iterator>
  void insert(Iterator beg, Iterator end) { return m_skeleton.insert(&m_arena, m_compare, beg, end); }
  void clear() { m_skeleton.clear(&m_arena); }
  bool empty() const { return m_skeleton.empty(); }
  size_t size() const { return m_arena.size(); }

	void acquire() const { m_skeleton.acquire(); }
	void release() const { m_skeleton.release(); }

  explicit tree_set_t(allocator_t* allocator, size_t page_size=DEFAULT_PAGE_SIZE, const comparator_type& compare=less_t<key_type>())
		: m_arena(allocator, sizeof(item_type), page_size), m_compare(compare) {}

  ~tree_set_t() { clear(); }

  tree_set_t(const tree_set_t& other)
		: m_arena(other.m_arena.allocator(), sizeof(item_type), other.m_arena.page_size()), 
			m_compare(other.m_compare)
  {
		insert(other.begin(), other.end());
  }
  
  tree_set_t& operator=(const tree_set_t& other)
  {
		tree_set_t other_copy = other;
		other_copy.exchange(*this);
		return *this;
  }

	template<class Synchronized>
  void exchange(tree_set_t& other, const Synchronized& sync)
  {
		if (this == &other)  { return; } // ESSENTIAL: deadlock in such case

		m_arena.exchange(other.m_arena, sync);
		m_skeleton.exchange(other.m_skeleton, sync);
		lock_scope_t<self_type, Synchronized> tl(this);
		lock_scope_t<self_type, Synchronized> ol(&other);
		unfact::exchange(m_compare, other.m_compare);
  }

  void exchange(tree_set_t& other) { exchange(other, synchronized_t()); }

  bool invariant() const { return m_skeleton.invariant(); }
  allocator_t* allocator() const { return m_arena.allocator(); }
  
public: // implementation detail: export just for testing and debuggin

	template<class Synchronized>
  size_t count(const Synchronized& sync) const
	{
		size_t n = m_skeleton.count(sync);
		size_t m = m_arena.size(sync);
		UF_ASSERT(n == m);
		return n;
	}

  size_t count() const { return count(synchronized_t()); }

private:
  arena_type m_arena;
  comparator_type m_compare;
  skeleton_type m_skeleton;
};

UNFACT_NAMESPACE_END

#endif//UNFACT_TREE_SET_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
