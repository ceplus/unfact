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

#ifndef UNFACT_RED_BLACK_HPP
#define UNFACT_RED_BLACK_HPP

#include <unfact/base.hpp>
#include <unfact/meta.hpp>
#include <unfact/concurrent.hpp> // for aquire_lock() and release_lock()

UNFACT_NAMESPACE_BEGIN

/*
 * red-black tree based balanced binary-tree implementation.
 * the algorithm is based on Cormen's "Introduction to Algorithm" 2ed, chapter 13. 
 *
 * red_black_t give only basic data structure and algorithm based on it.
 * so node allocation is user's task. 
 *
 * as std::set, item ordering is specified by given comparator.
 * it should be semi-reflective; e.g. (false == (compara(a,b) && compare(b,a)))
 *
 * thread-safety:
 * - red_black_t is NOT thread-safe. so synchronization is user's responsibility.
 *   wrapper classes like tree_set and set_tree does it.
 * - red_black_iterator_base and red_black_dfs_iterator are also NOt thread-safe,
 *    and because they hold external refennce,
 *   situation gets complicated.
 *   - some method  are 'local': we can use it safely it the thread owns the iterator
 *   - some methods are 'global': because they have external references, 
 *     you should synchronize entire rb-tree before access, even if you own the iterator.
 *   - see the comment on each method to see it is whether local or global.
 */

/* std::less equivalent */
template<class Key>
class less_t
{
public:
  typedef Key key_type;
  bool operator()(const key_type& x, const key_type& y) const { return x < y; }
};

/*
 * internal node of the rb-tree
 */
template<class Key, class Comparator=less_t<Key>, class Subclass=none_t>
struct red_black_t
{
  typedef Key key_type;
  typedef Comparator comparator_type;
  typedef red_black_t real_type;
  typedef typename select_type_t<Subclass, real_type>::type self_type;
  
  enum color_e
  {
		color_red = 0,
		color_black,
		colors
  };

  struct holder_t
  {
		holder_t() : m_left(0), m_right(0), m_up(0), m_color(color_red) {}

		self_type*  m_left;
		self_type*  m_right;
		self_type*  m_up;
		color_e  m_color; // we can save m_color by using bit-mask hack, but we don't do so at now...
  };

  holder_t m_holder;
  key_type m_key;

	template<class NewKey>
  explicit red_black_t(const NewKey& k=key_type()): m_key(k) {}

  self_type* self() { return static_cast<self_type*>(this); }
  const self_type* self() const { return static_cast<const self_type*>(this); }

  self_type* left() const { return m_holder.m_left; }
  self_type* right() const { return m_holder.m_right; }
  self_type* up() const { return m_holder.m_up; }
  color_e color() const { return m_holder.m_color; }
  const key_type& key() const { return m_key; }
  key_type& key() { return m_key; }
  const holder_t& holder() const { return m_holder; }
  void set_left(self_type* n) { m_holder.m_left = n; }
  void set_right(self_type* n) { m_holder.m_right = n; }
  void set_up(self_type* n) { m_holder.m_up = n; }
  void set_color(color_e c) { m_holder.m_color = c; }
  void set_key(const key_type& k) { m_holder.m_key = k; }
  void set_holder(const holder_t& other) {  m_holder = other;	}

  // shortcuts
  void set_black() { set_color(color_black); } 
  void set_red() { set_color(color_red); }
  bool black() const { return color_black == m_holder.m_color; }
  bool red() const { return color_red == m_holder.m_color; }

  bool rooted() const { return 0 == up(); }
  /* note that root nodes are never chained */
  bool chained() const { return 0 != up() || 0 != left() || 0 != right(); }

  size_t size() const
  {
		size_t n = 0;
		for (const self_type* i = minimum(); 0 != i; i = i->next()) { n++; }
		return n;
  }

  self_type* child() const
  {
		UF_ASSERT((0 == left()) || (0 == right()));
		return left() ? left() : right();
  }

  self_type* minimum()
  {
		self_type* here = this->self();
		while (here->left()) {
			here = here->left();
		}
		return here;
  }

  self_type* maximum()
  {
		self_type* here = this->self();
		while (here->right()) {
			here = here->right();
		}

		return here;
  }

  self_type* root()
  {
		self_type* here = this->self();
		while (here->up()) {
			here = here->up();
		}
	
		return here;
  }

  struct move_next_t
  {
		static self_type* r(self_type* that) { return that->right(); }
		static self_type* m(self_type* that) { return that->minimum(); }
  };

  struct move_prior_t
  {
		static self_type* r(self_type* that) { return that->left(); }
		static self_type* m(self_type* that) { return that->maximum(); }
  };

  template<class Move>
  self_type* move()
  {
		typedef Move move_type;

		if (0 != move_type::r(this->self())) {
			return move_type::m(move_type::r(this->self()));
		} else {
			self_type* last = this->self();
			self_type* here = this->up();
			while (here && move_type::r(here) == last) {
				last = here;
				here = here->up();
			}
			return here;
		}
  }

  self_type* next() { return move<move_next_t>(); }
  self_type* prior() { return move<move_prior_t>(); }

  template<class FindKey>
  self_type* find(const FindKey& key, const comparator_type& compare=comparator_type())
  {
		self_type* here = this->self();
		while (here) {
	  
			if (compare(key, here->key())) {
				here = here->left();
			} else {
				if (!compare(here->key(), key)) {
					return here;
				}
		  
				here = here->right();
			}
		}

		return 0;
  }

  const self_type* find(const Key& key, const comparator_type& compare=comparator_type()) const
  {
		return const_cast<real_type*>(this)->find(key, compare);
  }

  const self_type* next() const { return const_cast<real_type*>(this)->next(); }
  const self_type* prior() const { return const_cast<real_type*>(this)->prior(); }
  const self_type* root() const { return const_cast<real_type*>(this)->root(); }
  const self_type* minimum() const { return const_cast<real_type*>(this)->minimum(); }
  const self_type* maximum() const { return const_cast<real_type*>(this)->maximum(); }

  /*
   * Cormen's algorithm assumes null nodes as black (refered as 'sentinel' in the textbook)
   */
  static color_e color_for(const self_type* node)
  {
		return node ? node->color() : color_black;
  }
	
  /*
   * roatate() is symmetry operation. so we extract is mirror part to 
   * two trait classes: left_rotation_t and right_rotation_t
   */
  struct left_rotation_t
  {
		static self_type* l(self_type* n) { return n->left(); }
		static self_type* r(self_type* n) { return n->right(); }
		static void set_l(self_type* n, self_type* m) { return n->set_left(m); }
		static void set_r(self_type* n, self_type* m) { return n->set_right(m); }
  };

  struct right_rotation_t
  {
		static self_type* l(self_type* n) { return n->right(); }
		static self_type* r(self_type* n) { return n->left(); }
		static void set_l(self_type* n, self_type* m) { return n->set_right(m); }
		static void set_r(self_type* n, self_type* m) { return n->set_left(m); }
  };

  template<class Rotation>
  static void rotate(self_type* node)
  {
		typedef Rotation rot_type;
		self_type* next_root = rot_type::r(node);
		self_type* next_root_l = rot_type::l(next_root);
		rot_type::set_r(node, next_root_l);

		/*
		 * rechain backlefs
		 */
		if (next_root_l) { next_root_l->set_up(node); }
		if (node->up()) { node->rechain_up_backref(next_root); }

		/*
		 * chain the new arc
		 */
		rot_type::set_l(next_root, node);
		next_root->set_up(node->up());
		node->set_up(next_root);
  }

  static void rotate_left(self_type* node) { rotate<left_rotation_t>(node); }
  static void rotate_right(self_type* node) { rotate<right_rotation_t>(node); }

  /*
   * traits for forward_balance_insertion(), (similar to rotate())
   *
   */

  struct left_balance_t : public left_rotation_t
  {
		static void rotate_l(self_type* n) { return rotate_left(n); }
		static void rotate_r(self_type* n) { return rotate_right(n); }
  };

  struct right_balance_t : public right_rotation_t
  {
		static void rotate_l(self_type* n) { return rotate_right(n); }
		static void rotate_r(self_type* n) { return rotate_left(n); }
  };

  /*
   * @return true if new node is inserted successfully, 
   *         or false when the same key is already there.
   */
  static bool insert_unbalanced(self_type* root, self_type* node, 
																const comparator_type& compare=comparator_type())
  {
		self_type* here = root;
		self_type* last = 0;
		bool    last_compare = false;

		while (here) {
			last = here;
			last_compare = compare(node->key(), here->key());
			here = last_compare ? here->left() : here->right();
		}
	  
		if (last) {
			if (last_compare) {
				last->set_left(node);
			} else{
				if (!compare(last->key(), node->key())) {
					/* found same key: insertion failed */
					return false;
				}

				last->set_right(node);
			}
		}

		node->set_up(last);

		return true;
  }

  template<class Balance>
  static self_type* forward_balance_insertion(self_type* node)
  {
		typedef Balance bal_type;

		self_type* y = bal_type::r(node->up()->up());
		if (color_for(y) == color_red) {
			y->set_color(color_black);
			node->up()->set_color(color_black);
			node->up()->up()->set_color(color_red);
			node = node->up()->up();
		} else {
			if (node == bal_type::r(node->up())) {
				node = node->up();
				bal_type::rotate_l(node);
			}

			node->up()->set_color(color_black);
			node->up()->up()->set_color(color_red);
			bal_type::rotate_r(node->up()->up());
		}

		return node;
  }

  static void balance_insertion(self_type* root, self_type* node, self_type** new_root)
  {
		self_type* n = node;
		while (color_for(n->up()) == color_red) {
			if (n->up() == n->up()->up()->left()) {
				n = forward_balance_insertion<left_balance_t>(n);
			} else {
				n = forward_balance_insertion<right_balance_t>(n);
			}
		}

		/* actual root may have changed */
		self_type* nr = root->root();
		nr->set_color(color_black);
		*new_root = nr;
  }

  static bool insert(self_type* root, self_type* node, self_type** new_root, const comparator_type& compare=comparator_type())
  {
		UF_ASSERT(root && node && root->rooted() && (root != node) && !node->chained());
		bool ok = insert_unbalanced(root, node, compare);
		if (!ok) {
			return false;
		}

		balance_insertion(root, node, new_root);

		return true;
  }

  /* 
   * side-effect marker for remove_unbalanced()
   * 
   * original pseudo-code on the textbook return only "child" node, that is possibly nil-sentinel.
   * they use dirty trick to set up of sentinel. but we can not do such. so
   * return both child and up instead...
   */
  struct removal_unbalance_t
  {
		removal_unbalance_t() : m_to_rebalance(false), m_child(0), m_up(0) {}

		bool m_to_rebalance;
		self_type* m_child;  // child of removed node: may be null
		self_type* m_up; // up of removed node: may be null

		bool to_rebalance() const { return m_to_rebalance && (m_child || m_up); }

		self_type* root() const
		{
			if (m_up) return m_up->root();
			if (m_child) return m_child->root();
			return 0;
		}

  };

  static void remove_unbalanced(self_type* root, self_type* node, removal_unbalance_t* unbalance)
  {
		if ((!node->left()) || (!node->right())) { /* if has at most one child */
			/* rechain a child */
			self_type* child = node->child();
			self_type* chpar = node->up();
			if (child) {
				child->set_up(chpar);
			}

			/* rechain a up */
			if (chpar) {
				node->rechain_up_backref(child);
			}

			unbalance->m_to_rebalance = (color_black == node->color());
			unbalance->m_child = child;
			unbalance->m_up = chpar;
		} else { /* or has two children */
			self_type* toswap = node->next();
			UF_ASSERT(!toswap->left()); // consult Cormen's book to ask why this assumption is OK.
			remove_unbalanced(root, toswap, unbalance);

			/* rechain back references */
			if (node->left()) { node->left()->set_up(toswap); }
			if (node->right()) { node->right()->set_up(toswap); }
			if (node->up()) { node->rechain_up_backref(toswap); }
			if (unbalance->m_up == node) { unbalance->m_up = toswap; }
			/* then replace foward ref */
			toswap->set_holder(node->holder());
		}

		node->set_holder(holder_t());
  }

  template<class Balance>
  static self_type* forward_balance_removal(self_type* up)
  {
		typedef Balance bal_type;

		self_type* x = 0;
		self_type* w = bal_type::r(up);

		if (color_for(w) == color_red) {
			w->set_color(color_black);
			up->set_color(color_red);
			bal_type::rotate_l(up);
			w = bal_type::r(up);
		}

		if (color_for(bal_type::l(w)) == color_black &&
				color_for(bal_type::r(w)) == color_black) {
			w->set_color(color_red);
			x = up;
		} else {
			if (color_for(bal_type::r(w)) == color_black) { 
				bal_type::l(w)->set_color(color_black);
				w->set_color(color_red);
				bal_type::rotate_r(w);
				w = bal_type::r(up); 
			}

			w->set_color(color_for(up));
			up->set_color(color_black);
			bal_type::r(w)->set_color(color_black);
			bal_type::rotate_l(up);
			x = up->root();
		}

		return x;
  }

  static void balance_removal(self_type* node, self_type* up, self_type** new_root)
  {
		self_type* p = up;
		self_type* x = node;
		while (0 != p && color_for(x) == color_black) {
			if (x == p->left()) {
				x = forward_balance_removal<left_balance_t>(p);
				p = x->up();
			} else {
				x = forward_balance_removal<right_balance_t>(p);
				p = x->up();
			}
		}

		x->set_color(color_black);
		*new_root = x->root(); /* actual root may have changed */
  }

  static void remove(self_type* root, self_type* node, self_type** new_root)
  {
		UF_ASSERT(root && node && root->rooted() && ((root == node) || node->chained()));

		removal_unbalance_t unbalance;
		remove_unbalanced(root, node, &unbalance);
		if (unbalance.to_rebalance()) {
			balance_removal(unbalance.m_child, unbalance.m_up, new_root);
		} else {
			*new_root = unbalance.root();
		}
  }
	

  /* used by invariant() internally */
  struct branch_invariant_t
  {
		branch_invariant_t(const self_type* node, const comparator_type& compare)
			: m_satisfied(false), m_nblack(0)
		{
			/* all leaves should be black */
			if (0 == node) {
				m_satisfied = true;
				m_nblack = 1; // null is assumed to be black(leaf)
				return;
			}

			if (color_for(node) == color_red &&
					(color_black != color_for(node->left()) ||
					 color_black != color_for(node->right()))) {
				UF_ALERT(("if the node is red, both of its children should be black!"));
				return;
			}

			if (node->left() && compare(node->key(), node->left()->key())) {
				UF_ALERT(("left child is greater than up!"));
				return;
			}

			if (node->right() && compare(node->right()->key(), node->key())) {
				UF_ALERT(("left child is less than up!"));
				return;
			}

			branch_invariant_t l(node->left(), compare);
			branch_invariant_t r(node->right(), compare);
			if (!(l.m_satisfied) || !(r.m_satisfied)) {
				UF_ALERT(("both children should be satisfied in nature!"));
				return;
			}

			if (l.m_nblack != r.m_nblack) {
				UF_ALERT(("for each node, count of black nodes on the paths to both children should be equal! %d:%d",
									l.m_nblack, r.m_nblack));
				return;
			}

			m_satisfied = true;
			m_nblack = l.m_nblack + (color_for(node) == color_black ? 1 : 0);
			return;
		}

		bool satisfied() const { return m_satisfied; }

  private:
		bool   m_satisfied;
		size_t m_nblack;
  };

  /*
   * utility method to inspect rb-tree invariant (as root).
   * intend to use at debugging. so impelementation is not efficient.
   */
  bool invariant(const comparator_type& compare=comparator_type()) const
  {
		UF_ALERT_AND_RETURN_UNLESS(rooted(), false, 
															 "invaiant should be checked against root node!");
		UF_ALERT_AND_RETURN_UNLESS(color() == color_black, false, 
															 "root node should be black!");
		branch_invariant_t bi(this->self(), compare);
		return bi.satisfied();
  }

private:
  void rechain_up_backref(self_type* next)
  {
		if (this == this->up()->left()) {
			this->up()->set_left(next);
		} else {
			UF_ASSERT(this == this->up()->right());
			this->up()->set_right(next);
		}
  }

private:
  red_black_t(const red_black_t&);
  const red_black_t& operator=(const red_black_t&);
};

/*
 * helper object for implementing iterator over red_black_t structure.
 * this helper provide only traversal mechanism.
 * you should implement deferecne methods like op* and op->
 */
template<class Subclass, class Key, class Comparator, class RbSubclass=none_t>
class red_black_iterator_base_t
{
public:
  typedef red_black_iterator_base_t<Subclass, Key, Comparator, RbSubclass> self_type;
  typedef typename red_black_t<Key, Comparator, RbSubclass>::self_type node_type;

  explicit red_black_iterator_base_t(node_type* n) : m_node(n) {}

	/* thead-model: local */
  bool operator==(const self_type& other) const { return node() == other.node(); }
	/* thead-model: local */
  bool operator!=(const self_type& other) const { return node() != other.node(); }
	/* thead-model: global */
  Subclass& operator++() { do_increment(); return static_cast<Subclass&>(*this); }
	/* thead-model: global */
  Subclass  operator++(int) { Subclass ret = static_cast<Subclass&>(*this); do_increment(); return ret; }
	/* thead-model: global */
  Subclass& operator--() { do_decrement(); return static_cast<Subclass&>(*this); }
	/* thead-model: global */
  Subclass  operator--(int) { Subclass ret = static_cast<Subclass&>(*this); do_decrement(); return ret; }

	/* thead-model: local */
  bool atend() const { return 0 == m_node; }
	/* thead-model: local */
  bool good() const { return !atend(); }

	void acquire() const { acquire_lock(m_node); }
	void release() const { release_lock(m_node); }

public: // for internal use
  void increment() { m_node = m_node->next(); }
  void decrement() { m_node = m_node->prior(); }
  node_type* node() const { return m_node; }
  void set_node(node_type* n) { m_node = n; }
  void clear() { m_node = 0;}

  /*
   * hack to use increment()/decrement() implementation on subclass if it is available.
   * in this way we avoid overhead on dynamic polymorphism.
   */
  void do_increment() { return static_cast<Subclass*>(this)->increment(); }
  void do_decrement() { return static_cast<Subclass*>(this)->decrement(); }

private:
  node_type* m_node;
};

/*
 * depth-first traversal for red-black tree.
 *
 * This iterator visit leaf first, then go their ups.
 * That is because the iterator is originally designed for
 * recursive node removal on tree destructor, where leaf-then-up policy is suited for.
 */
template< class Key, class Comparator=less_t<Key>, class Subclass=none_t>
class red_black_dfs_iterator_t
{
public:
  typedef red_black_dfs_iterator_t self_type;
  typedef typename red_black_t<Key, Comparator, Subclass>::self_type node_type;
  typedef Key  value_type;
  typedef Key* pointer_type;

  red_black_dfs_iterator_t(node_type* n)
		: m_node(minimum_leaf(n)) {
  }
  
	/* thead-model: local */
  bool operator==(const self_type& other) { return node() == other.node(); }
	/* thead-model: local */
  bool operator!=(const self_type& other) { return node() != other.node(); }
	/* thead-model: global */
  self_type& operator++() { increment(); return *this; }
	/* thead-model: global */
  self_type  operator++(int) { self_type ret = *this; increment(); return ret; }
	/* thead-model: global */
  self_type& operator--() { decrement(); return *this; }
	/* thead-model: global */
  self_type  operator--(int) { self_type ret = *this; decrement(); return ret; }

	/* thead-model: local */
  value_type& operator*() const { return m_node->key(); }
	/* thead-model: local */
  pointer_type operator->() const { return &(m_node->key()); }

	/* thead-model: local */
  bool atend() const { return 0 == m_node; }
	/* thead-model: local */
  operator bool () const { return !atend(); }

	/* thead-model: local */
  node_type* node() const { return m_node; }

public: // for internal use
  static node_type* minimum_leaf(node_type* node) {
		if (!node) {
			return 0;
		} else {
			node_type* i = node;
			while (i->left() || i->right()) { i = i->left() ? i->left() : i->right(); }
			return i;
		}
  }

  void increment()
  {
		UF_ASSERT(m_node);
		node_type* n = m_node;
		node_type* p = n->up();
		if (!p) {
			m_node = 0;
		} else {
			if (p->left() == n && p->right()) {
				m_node = minimum_leaf(p->right());
			} else {
				m_node = p;
			}
		}
  }

  void decrement() 
  {
		UF_ASSERT(!"Not Impelmented Yet");
  }

private:
  node_type* m_node;
};

UNFACT_NAMESPACE_END

#endif//UNFACT_RED_BLACK_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   tab-width: 2
   End:
   -*- */
