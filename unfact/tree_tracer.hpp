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

#ifndef UNFACT_TREE_TRACER_HPP
#define UNFACT_TREE_TRACER_HPP

#include <unfact/static_string.hpp>
#include <unfact/set_tree.hpp>
#include <unfact/keyed_value.hpp>

UNFACT_NAMESPACE_BEGIN

/*
 * tree_tracer is a stack-like abstraction for tracing tree.
 *
 * During tracing, we incrementaly build a 'tracing tree', a call-tree like structure.
 * We call each node of tracing tree 'scope'.
 * We can call push() when we enter the some distinguished segment - scope -,
 * then tree_tracer build a new child node for the segment, or lookup and traverse into an existing child for it.
 * We call 'pop()' to leave the scope, respectively.
 * Each scope has their 'name'. name is a string identifier that is used to print tracing statistics. 
 * 'Ticket' is a identifier of each scope. that is returned from push() and pop(). 
 * We can use 'at()' to dereference the scope entity. 
 *
 * Although push() and pop() is a primary operation for the tracer, 
 * we also provide tree traversal api like parent(), begin(), end(), etc.
 * These APIs are useful when making the statistics report. 
 */
template<class Value, class Concurrent=null_concurrent_t>
class tree_tracer_t
{
public:
  typedef basic_static_string_t<char> key_type;
  typedef Value value_type;
	typedef Concurrent concurrent_type;
	typedef tree_tracer_t self_type;
	typedef basic_arena_t<concurrent_type> arena_t;
  typedef keyed_value_t<key_type, value_type> node_type;
  typedef set_tree_t<node_type, less_t<node_type>, concurrent_type> tree_type;
  typedef typename tree_type::ticket_t ticket_type;
  typedef typename tree_type::const_iterator iterator;

	// impl detail...
	typedef typename tree_type::const_child_iterator_t const_child_iterator_type;

  enum { key_size = key_type::capacity  };
  
  tree_tracer_t(allocator_t* allocator, size_t page_size=DEFAULT_PAGE_SIZE)
		: m_tree(node_type(""), allocator, page_size) {}

	/*
	 * thread safety:
	 * begin(), end(), parent() are stateless
	 */
  iterator begin() const { return m_tree.begin(); }
  iterator end() const { return m_tree.end(); }
  iterator parent(iterator here) const { return tree_type::parent(here); }

	/*
	 * thread safety:
	 * begin_for() and end_for() are thread safe, but stateful
	 */
	template<class Synchronized>
  iterator begin_for(ticket_type here, const Synchronized& sync) const { return m_tree.begin_for(tree_type::to_const_child_iterator(here), sync); }
  iterator begin_for(ticket_type here) const { return begin_for(here, synchronized_t()); }
	template<class Synchronized>
  iterator end_for(ticket_type here, const Synchronized& sync) const { return m_tree.end_for(tree_type::to_const_child_iterator(here), sync); }
  iterator end_for(ticket_type here) const { return end_for(here, synchronized_t()); }

	/*
	 * to_xxx() family are stateless
	 */
  static iterator to_iterator(ticket_type t) { return tree_type::to_const_iterator(t); }
  static ticket_type to_ticket(iterator i) { return tree_type::to_ticket(i); }

	/*
	 * following ticket accessors are thread-safe, although most of them are stateful.
	 */
  ticket_type root() const
  {
		return tree_type::to_ticket(m_tree.child_begin());
  }

  ticket_type push(ticket_type parent, const key_type& name)
  {
		return tree_type::to_ticket(m_tree.ensure(tree_type::to_child_iterator(parent), node_type(name)));
  }

  ticket_type pop(ticket_type top)
  {
		return tree_type::to_ticket(parent(tree_type::to_iterator(top)));
  }

  ticket_type parent(ticket_type here) const
  {
		return to_ticket(parent(to_iterator(here)));
  }

  /*
   * node accessors : operation is iterator-local. it just dereference iterator.
   */
  const key_type& name_of(ticket_type ticket) const { return to_iterator(ticket)->key(); }
  const value_type& at(ticket_type ticket) const { return to_iterator(ticket)->value(); }
  value_type& at(ticket_type ticket) { return tree_type::to_child_iterator(ticket)->value(); }

	template<class Synchronized>
	void fill(const value_type& val, const Synchronized& sync)
	{
		fill(root(), val, sync);
	}

	void fill(const value_type& t) { fill(t, synchronized_t()); }

	template<class Synchronized>
	void fill(ticket_type here, const value_type& val, const Synchronized&)
	{
		typedef typename tree_type::iterator iter_type;
		for(iter_type
					i=m_tree.begin_for(tree_type::to_iterator(here)),
					e=m_tree.end_for(tree_type::to_iterator(here));
					i!=e; /* */) {
			(*lock_scope_t<iter_type, Synchronized>(&i))->set_value(val);
			++i;
		}
	}

	void fill(ticket_type here, const value_type& t) { fill(here, t, synchronized_t()); }

  void format_name(iterator here, char* buf, size_t bufsize, size_t* written) const
  {
		size_t left = bufsize;

		iterator parent = this->parent(here);
		if (parent.good()) {
			size_t parent_written = 0;
			format_name(parent, buf, bufsize, &parent_written);
			left -= parent_written;
			if (left <= 1) { // filled
				*written = bufsize - left;
				return;
			}
	  
			if (0 < parent_written) {

				if (2 <= left) {
					buf[bufsize-left+0] = '.';
					buf[bufsize-left+1] = '\0';
					/* NOTE: we does not count '\0' as consumed because caller can overwrite it freely */
					left -= 1;
				}

				if (left <= 1) { // filled
					*written = bufsize - left;
					return; 
				}
			}
		}

		size_t name_size = here->key().size();
		if (name_size < left) {
			string_ops_t<char>::copy(buf + (bufsize-left), left, here->key().c_str());
			left -= min_of(name_size, left);
		}

		*written = bufsize - left;
		return;
	}

	void acquire() const { m_tree.acquire(); }
	void release() const { m_tree.release(); }

private:
  tree_type m_tree;
};

UNFACT_NAMESPACE_END

#endif//UNFACT_TREE_TRACER_HPP

/* -*-
	 Local Variables:
	 mode: c++
	 c-tab-always-indent: t
	 c-indent-level: 2
	 c-basic-offset: 2
	 tab-width: 2
	 End:
	 -*- */
