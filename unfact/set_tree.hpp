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

#ifndef UNFACT_SET_TREE_HPP
#define UNFACT_SET_TREE_HPP

#include <unfact/base.hpp>
#include <unfact/arena.hpp>
#include <unfact/algorithm.hpp>
#include <unfact/red_black.hpp>
#include <unfact/tree_set.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * set_tree is recursive set structure designed for make filesystem-like namespace
 * set_tere consists of set of element, that itself is a set of same type of element.
 * in other words, set_tree is set of set of set of ... of set.
 * image filesystem. its directory is set, that may contain another directory. 
 * set_tree is based on similar idea.
 *
 * thread safety:
 * - TBD
 *
 * IDEA: currently node on set_tree is 7 word : it seems large a bit.
 *       we can implement ordered_chain_t (that has same interface to red_black_t) and 
 *       parameterize node implementation to save a memory
 */

/*
 * set node:
 * it hold children set (m_children) and back-reference to containing node (m_parnt)
 *
 * thread-safety:
 * nested_red_black_t is NOT thread safe as its superclass,
 * but the child_set_type instances retuned by child() may be safe.
 * acquire() and release() is just delegated to them.
 */
template<class Key, class Comparator=less_t<Key>, class Concurrent=null_concurrent_t>
struct set_tree_node_t : 
	public red_black_t< Key, Comparator, set_tree_node_t<Key, Comparator, Concurrent> >
{
  typedef Key key_type;
  typedef Comparator comparator_type;
	typedef Concurrent concurrent_type;

	typedef red_black_t<key_type, comparator_type, set_tree_node_t> base_type;
	typedef tree_set_skeleton_t<key_type, comparator_type, set_tree_node_t, concurrent_type> child_set_type;
	typedef typename base_type::self_type self_type;

  typedef typename child_set_type::iterator_t child_iterator_t;
  typedef typename child_set_type::const_iterator_t const_child_iterator_t;

	struct initializer_t
	{
		initializer_t(const key_type& key, self_type* parent) : m_key(key), m_parent(parent) {}
		const key_type& key() const { return m_key; }
		self_type* parent() const { return m_parent; }
		operator const key_type&() const { return m_key; } // for comparison
	private:
		key_type m_key;
		self_type* m_parent;
	};

	explicit set_tree_node_t(const initializer_t& init)
		: base_type(init.key()), m_parent(init.parent()) {}

	/* following accessors are stateless (although its content is not) */
	self_type* parent() const { return m_parent; }
	child_set_type& children() { return m_children; }
	const child_set_type& children() const { return m_children; }

	void acquire() const { m_children.acquire(); }
	void release() const { m_children.release(); }
private:
	self_type* m_parent;
	child_set_type m_children;
};

/*
 * Whole traversal iterator for set-tree: this is designed for stat analysis and reporting.
 * so there are no mutators - you can just visit and inspect the value, but not mofidy them.
 *
 * Unlike other iterators, traverse set_tree via set_tree_iterator_t is thread-safe.
 * If you lock the set_tree before using the iterator, you should use _nolock family instead.
 * '++' operator can/should be altered with increment_nolock() method.
 *
 * TODO: make full traversal iterator and wrap it.
 */
template<class Key, class Comparator, class Concurrent, class Subclass>
class set_tree_iterator_base_t
	: public red_black_iterator_base_t< Subclass, Key, Comparator, 
																			set_tree_node_t<Key, Comparator, Concurrent> >
{
public:
  typedef Key key_type;
  typedef Comparator comparator_type;
	typedef Concurrent concurrent_type;
	typedef Subclass subclass_type;
	typedef set_tree_iterator_base_t self_type;
	typedef set_tree_node_t<key_type, comparator_type, concurrent_type> node_type;
	typedef red_black_iterator_base_t<subclass_type, key_type, comparator_type, node_type> base_type;

	template <class Iterator>
	explicit set_tree_iterator_base_t(Iterator i) : base_type(i.node()) {}
	explicit set_tree_iterator_base_t(node_type* n) : base_type(n) {}
	set_tree_iterator_base_t() : base_type(0) {}

	size_t height() const
	{
		size_t h = 0;
		for (const node_type* n = base_type::node(); 0 != n; n = n->parent()) {
			h++;
		}

		return h;
	}

	/*
	 * begin_for() and friends are factory methods for child tree.
	 * synchronous version need extra lock because root node has no parent, which we use as a lock.
	 */
	template<class Synchronized>
	static subclass_type begin_for(node_type* n, const Synchronized& sync) { return subclass_type(first_leaf(n, sync)); }
	static subclass_type begin_for(node_type* n) { return begin_for(n, synchronized_t()); }
	template<class Synchronized>
	static subclass_type end_for(node_type* n, const Synchronized& sync) { return subclass_type(next(n, sync)); }
	static subclass_type end_for(node_type* n) { return end_for(n, synchronized_t()); }

public: // implementation detail

	template<class Synchronized>
	void increment(const Synchronized& sync) { base_type::set_node(next(base_type::node(), sync)); }
	void increment() { increment(synchronized_t()); }

	template<class Synchronized>
	void decrement(const Synchronized&) { UF_NOT_IMPLEMENTED_YET(); }
	void decrement() { UF_NOT_IMPLEMENTED_YET(); }

	template<class Synchronized>
	static node_type* first_leaf(node_type* node, const Synchronized&)
	{
		typedef typename node_type::const_child_iterator_t iter_type;

		node_type* n = 0;
		iter_type i(node);
		iter_type next;
		do {
			n = i.node();
			lock_scope_t<node_type, Synchronized> l(n);
			i = n->children().begin(unsynchronized_t());
		}	while (i.good());

		return n;
	}

	static node_type* first_leaf(node_type* node) {	return first_leaf<synchronized_t>(node); }

	template<class Synchronized>
	static node_type* next(node_type* node, const Synchronized& sync)
	{
		node_type* p = node->parent(); // note that parent() is stateless
		node_type* next = 0;
		if (!p) {
			/* if the node has no parent, he/she should have no siblings. */
			return 0;
		} else {
			lock_scope_t<node_type, Synchronized> l(p);
			next = node->next();
			// release the lock here.
		}

		if (next) {
			return first_leaf(next, sync);
		} else {
			return p;
		}
	}

	static node_type* next(node_type* node) { return next(node, synchronized_t()); }   

};

template<class Key, class Comparator, class Concurrent>
class const_set_tree_iterator_t : public set_tree_iterator_base_t<Key, Comparator, Concurrent, 
																																	const_set_tree_iterator_t<Key, Comparator, Concurrent> >
{
public:
	typedef const_set_tree_iterator_t self_type;
	typedef const set_tree_iterator_base_t<Key, Comparator, Concurrent, self_type> base_type;
	typedef typename base_type::key_type key_type;
	typedef typename base_type::node_type node_type;
	typedef const key_type  value_type;
	typedef const key_type* pointer_type;

	template <class Iterator>
	explicit const_set_tree_iterator_t(Iterator i) : base_type(i) {}
	explicit const_set_tree_iterator_t(node_type* n) : base_type(n) {}
	const_set_tree_iterator_t() {}

	value_type& operator*() const { return base_type::node()->key(); }
	pointer_type operator->() const { return &(base_type::node()->key()); }
};

template<class Key, class Comparator, class Concurrent>
class set_tree_iterator_t : public set_tree_iterator_base_t<Key, Comparator, Concurrent, 
																														set_tree_iterator_t<Key, Comparator, Concurrent> >
{
public:
	typedef set_tree_iterator_t self_type;
	typedef const_set_tree_iterator_t<Key, Comparator, Concurrent> const_iterator_type;
	typedef const set_tree_iterator_base_t<Key, Comparator, Concurrent, self_type> base_type;
	typedef typename base_type::key_type key_type;
	typedef typename base_type::node_type node_type;
	typedef key_type  value_type;
	typedef key_type* pointer_type;

	template <class Iterator>
	explicit set_tree_iterator_t(Iterator i) : base_type(i) {}
	explicit set_tree_iterator_t(node_type* n) : base_type(n) {}
	set_tree_iterator_t() {}

	value_type& operator*() const { return base_type::node()->key(); }
	pointer_type operator->() const { return &(base_type::node()->key()); }

	operator const_iterator_type() const { return const_iterator_type(base_type::node()); } // down conversion
};

/*
 * collection body
 */
template<class Key, class Comparator=less_t<Key>, class Concurrent=null_concurrent_t>
class set_tree_t
{
public:
  typedef Key key_type;
  typedef Comparator comparator_type;
	typedef Concurrent concurrent_type;
	typedef set_tree_t self_type;
	typedef basic_arena_t<concurrent_type> arena_type;

	typedef set_tree_node_t<key_type, comparator_type, concurrent_type> node_type;
  typedef typename node_type::child_set_type child_set_type;
	typedef typename node_type::initializer_t node_initializer_type;

  class ticket_handle;
  typedef ticket_handle* ticket_t;
	class ticket_lock_scope_t; // defined below

  typedef typename node_type::child_iterator_t child_iterator_t;
  typedef typename node_type::const_child_iterator_t const_child_iterator_t;
  typedef red_black_dfs_iterator_t<key_type, comparator_type, node_type> dfs_iterator_type;

  /*
   * pseudo accessors to internal data structures behind iterator
   * more "formal" way inclues subclassing iterator classes. but it cause code bloat.
   * so we intentionally avoid it.
	 *
	 * children() is thread safe because i.node() is (almost) immutable,
	 * and is node::children() respectively.
   */
  static child_set_type& children(child_iterator_t& i) { return i.node()->children(); }
  static const child_set_type& children(const_child_iterator_t& i) { return i.node()->children(); }
  static child_set_type& children(dfs_iterator_type& i) { return i.node()->children(); }

	/* both are stateless */
  template<class Iterator>
  static node_type* parent_node(Iterator i) { return i.node()->parent(); }
  template<class Iterator>
  static Iterator parent(Iterator i) { return Iterator(i.node()->parent()); }

	typedef const_set_tree_iterator_t<key_type, comparator_type, concurrent_type> const_iterator_type;
	typedef set_tree_iterator_t<key_type, comparator_type, concurrent_type> iterator_type;
  /* iterator and const_iterator are for STL compatibility */
  typedef iterator_type iterator;
  typedef const_iterator_type const_iterator;

  /*
   * iterators
	 *
	 * thread-safety:
	 * child_begin(), child_end(), begin(), end() are stateless.
	 * note that begin_for(&m_root) and end_for(&m_root) are safe and stateless
	 * because m_root never belongs to any tree.
	 *
	 * xxx_for() family is stateful.
   */

	template<class Synchronized>
	const_iterator_type begin(const Synchronized& sync) const { return const_iterator_type::begin_for(const_cast<node_type*>(&m_root), sync); }
	const_iterator_type begin() const { return begin(synchronized_t()); }
	template<class Synchronized>
	iterator_type begin(const Synchronized& sync) { return iterator_type::begin_for(&m_root, sync); }
	iterator_type begin() { return begin(synchronized_t()); }
	
	template<class Synchronized>
  const_iterator_type end(const Synchronized& sync) const { return const_iterator_type::end_for(const_cast<node_type*>(&m_root), sync); }
	const_iterator_type end() const { return end(synchronized_t()); }
	template<class Synchronized>
  iterator_type end(const Synchronized& sync) { return iterator_type::end_for(&m_root, sync); }
	iterator_type end() { return end(synchronized_t()); }

	template<class Iterator, class Synchronized>
  const_iterator_type begin_for(Iterator i, const Synchronized& sync) const { return const_iterator_type::begin_for(i.node(), sync); }
	template<class Iterator>
  const_iterator_type begin_for(Iterator i) const { return begin_for(i, synchronized_t()); }
	template<class Iterator, class Synchronized>
  iterator_type begin_for(Iterator i, const Synchronized& sync) { return iterator_type::begin_for(i.node(), sync); }
	template<class Iterator>
  iterator_type begin_for(Iterator i) { return begin_for(i, synchronized_t()); }

	template<class Iterator, class Synchronized>
  const_iterator_type end_for(Iterator i, const Synchronized& sync) const { return const_iterator_type::end_for(i.node(), sync); }
	template<class Iterator>
  const_iterator_type end_for(Iterator i) const { return end_for(i, synchronized_t()); }
	template<class Iterator, class Synchronized>
  iterator_type end_for(Iterator i, const Synchronized& sync) { return iterator_type::end_for(i.node(), sync); }
	template<class Iterator>
  iterator_type end_for(Iterator i) { return end_for(i, synchronized_t()); }

  /*
   * iterator for children of certain node
   */
  child_iterator_t child_begin() { return child_iterator_t(&m_root); }
  child_iterator_t child_end() { return child_iterator_t(0); } // just for consistency
  const_child_iterator_t child_begin() const { return const_cast<self_type*>(this)->child_begin(); }
  const_child_iterator_t child_end() const { return const_cast<self_type*>(this)->child_end(); }

  template<class Iterator, class Synchronized>
  child_iterator_t child_begin(Iterator parent, const Synchronized& sync) { return children(parent).begin(sync); }
  template<class Iterator>
  child_iterator_t child_begin(Iterator parent) { return child_begin(parent, synchronized_t()); }
  template<class Iterator, class Synchronized>
  child_iterator_t child_end(Iterator parent, const Synchronized&) { return child_iterator_t(0); }
  template<class Iterator>
  child_iterator_t child_end(Iterator parent) { return child_end(parent, synchronized_t()); }
  template<class Iterator, class Synchronized>
  const_child_iterator_t child_begin_for(Iterator parent, const Synchronized& sync) const { return children(parent).begin(sync); }
  template<class Iterator>
  const_child_iterator_t child_begin_for(Iterator parent) const { return child_begin_for(parent, synchronized_t()); }
  template<class Iterator, class Synchronized>
  const_child_iterator_t child_end_for(Iterator /*parent*/, const Synchronized&) const { return const_child_iterator_t(0); }
  template<class Iterator>
  const_child_iterator_t child_end_for(Iterator parent) const { return child_end_for(parent, synchronized_t()); }

  /*
   * iterator <-> ticket conversion.
   * ticket is a iterator compatible opqaue type that fit into pointer size storage.
   * tickets are designed to store iterators into thread local storages.
   */ 
  template<class Iterator>
	static ticket_t to_ticket(Iterator iter) { return reinterpret_cast<ticket_t>(iter.node()); }
  static const_child_iterator_t to_const_child_iterator(ticket_t ticket) { return const_child_iterator_t(reinterpret_cast<node_type*>(ticket)); }
  static child_iterator_t to_child_iterator(ticket_t ticket) { return child_iterator_t(reinterpret_cast<node_type*>(ticket)); }
  static const_iterator_type to_const_iterator(ticket_t ticket) { return const_iterator_type(reinterpret_cast<node_type*>(ticket)); }
  static iterator_type to_iterator(ticket_t ticket) { return iterator_type(reinterpret_cast<node_type*>(ticket)); }

	// does need nolock?
  size_t size() const { return m_arena.size(); }
  bool empty() const { return 0 == size(); }
  size_t child_size(const_child_iterator_t iter) const { return child_count(iter); }
  bool child_empty(const_child_iterator_t iter) const { return children(iter).empty(); }


  template<class Iterator, class FindKey>
  child_iterator_t find(Iterator parent, const FindKey& key)
  {
		UF_HONOR_OR_RETURN(parent.good(), child_iterator_t(0));
		return children(parent).find(m_compare, key);
  }

  template<class Iterator, class FindKey>
  const_child_iterator_t find(Iterator parent, const FindKey& key) const
  {
		UF_HONOR_OR_RETURN(parent.good(), child_iterator_t(0));
		return children(parent).find(m_compare, key);
  }

  /*
   * @param Iterator should be child_iterator_t ro const_child_iterator_t
   * @return end() when there is already same key
   *         or iterator that points inserted key
   */
  template<class Iterator, class NewKey>
  child_iterator_t insert(Iterator parent, const NewKey& key)
  {
		UF_HONOR_OR_RETURN(parent.good(), child_iterator_t(0));
		return children(parent).insert
			(&m_arena, m_compare, node_initializer_type(key, parent.node())); 
  }

  /*
   * @param Iterator should be child_iterator_t ro const_child_iterator_t
   * @return existing one when key is already inserted,
   *         or new iterator that points inserted key
   */
  template<class Iterator, class NewKey>
  child_iterator_t ensure(Iterator parent, const NewKey& key)
  {
		UF_HONOR_OR_RETURN(parent.good(), child_iterator_t(0));
		return children(parent).ensure
			(&m_arena, m_compare, node_initializer_type(key, parent.node())); 
  }

	/*
	 * We assume you have exclusive access to the 'iter' node.
	 * concurrent access to the node causes undefined catastrophic behaviour.
	 */
  template<class Iterator>
  void remove(Iterator iter)
  {
		if (iter.atend()) {
			return;
		}

		clear(iter);
		parent_node(iter)->children().remove(&m_arena, iter);
  }

  void clear() { clear(child_begin()); }

  template<class Iterator>
  void clear(Iterator beg, Iterator end)
  {
		for (Iterator i=beg; i!=end; ++i) { clear(i); }
  }

	/*
	 * @todo: doc assumed concurrency restriction
	 */
  template<class Iterator, class Syncronized>
  void clear(Iterator iter, const Syncronized& sync)
  {
		child_set_type last_children;
		children(iter).exchange(last_children, sync);

		// unsynchronized: because we've detached the children and now exclusively own it.
		for (dfs_iterator_type i = last_children.dfs_begin(unsynchronized_t());
				 i != last_children.dfs_end(unsynchronized_t()); ++i) {
			clear(i, unsynchronized_t());
		}

		last_children.clear(&m_arena); 
  }

  template<class Iterator>
  void clear(Iterator iter) { clear(iter, synchronized_t()); }

	void acquire() const { m_arena.acquire(); }
	void release() const { m_arena.release(); }

  allocator_t* allocator() const { return m_arena.allocator(); }

  explicit set_tree_t(const Key& root_key, 
											allocator_t* allocator, 
											size_t page_size=DEFAULT_PAGE_SIZE, 
											const comparator_type& compare=less_t<key_type>())
		: m_arena(allocator, sizeof(node_type), page_size), m_compare(compare), 
			m_root(node_initializer_type(root_key, 0)) {}

  ~set_tree_t() { clear(); }
 
public: // implementation detail: export just for testing and debugging

  /* used from child_size() */
  template<class Iterator, class Synchronized>
  size_t child_count(Iterator iter, const Synchronized& sync) const {	return children(iter).count(sync); }
  template<class Iterator>
	size_t child_count(Iterator iter) const { return child_count(iter, synchronized_t()); }

  /*
   * traverse data sturucture recursively and count actual item count : useful for debugging.
	 *
	 * the number is inaccurate in the concurrent world,
	 * because not can be inserted or removed during counting.
   */
  size_t count() const {return descendant_count(child_begin()); }

	/*
	 * CAUTION:
	 * Here we break 'open call' rule what we obey here in the unfact.
	 * We assume the lock priority instead: 
	 * Parent lock has higher priority than one of its children. 
	 * Be careful. Deadlock can happen when we violate the priority!
	 */
	template<class Iterator, class Synchronized>
  size_t descendant_count(Iterator iter, const Synchronized& sync) const
  {
		lock_scope_t<Iterator, Synchronized> l(&iter);
		size_t n = child_count(iter, unsynchronized_t());
		for (const_child_iterator_t 
					 i=child_begin_for(iter, unsynchronized_t()), 
					 e=child_end_for(iter, unsynchronized_t());
				 i != e; ++i) {
			n += descendant_count(i, sync);
		}
	
		return n;
  }

	template<class Iterator>
  size_t descendant_count(Iterator iter) const { return descendant_count(iter, synchronized_t()); }

  const node_type* root() const { return &m_root; }

private:
  set_tree_t(const set_tree_t& other);
  set_tree_t& operator=(const set_tree_t& other);

private:
  arena_type m_arena;
  comparator_type m_compare;
  node_type m_root;
};

UNFACT_NAMESPACE_END

#endif//UNFACT_SET_TREE_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 tab-width: 2
	 End:
	 -*- */
