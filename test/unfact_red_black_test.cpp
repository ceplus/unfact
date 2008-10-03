#include <unfact/red_black.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

namespace uf = unfact;

#if 0
static void
test_tree_set_hello()
{
  tracing_allocator_t alloc;
  uf::tree_set_t<int> s(&alloc, 1024);
}
#endif

#define UF_TEST_TREE_SET_NODE_HAS(node, l, r, p) \
  UF_TEST_EQUAL((node)->right(),  r);			 \
  UF_TEST_EQUAL((node)->left(),   l);			 \
  UF_TEST_EQUAL((node)->up(),     p)

typedef uf::red_black_t<int> int_node_type;

static void
test_tree_set_empty_node()
{
  int_node_type n1(1);
  UF_TEST_EQUAL(n1.minimum(), &n1);
  UF_TEST_EQUAL(n1.next(), 0);
}

static void
test_tree_set_next_prior()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);

  UF_TEST(int_node_type::insert_unbalanced(&n1, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n4));

  UF_TEST_EQUAL(n1.next(), &n2);
  UF_TEST_EQUAL(n2.next(), &n3);
  UF_TEST_EQUAL(n3.next(), &n4);
  UF_TEST_EQUAL(n4.next(),   0);

  UF_TEST_EQUAL(n1.prior(),   0);
  UF_TEST_EQUAL(n2.prior(), &n1);
  UF_TEST_EQUAL(n3.prior(), &n2);
  UF_TEST_EQUAL(n4.prior(), &n3);
  
}

static void
test_tree_set_insert_unbalanced_ordered()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);

  UF_TEST_EQUAL(n1.minimum(), &n1);
  UF_TEST_EQUAL(n1.next(), 0);

  UF_TEST(int_node_type::insert_unbalanced(&n1, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n4));

  UF_TEST_TREE_SET_NODE_HAS(&n1, 0, &n2,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n2, 0, &n3, &n1);
  UF_TEST_TREE_SET_NODE_HAS(&n3, 0, &n4, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n4, 0,   0, &n3);

  UF_TEST_EQUAL(n1.minimum(), &n1);
  UF_TEST_EQUAL(n1.next(), &n2);
  UF_TEST_EQUAL(n2.next(), &n3);
  UF_TEST_EQUAL(n3.next(), &n4);
  UF_TEST_EQUAL(n4.next(),   0);

  UF_TEST_EQUAL(n1.find(0), 0);
  UF_TEST_EQUAL(n1.find(5), 0);
  UF_TEST_EQUAL(n1.find(1)->key(), 1);
  UF_TEST_EQUAL(n1.find(2)->key(), 2);
  UF_TEST_EQUAL(n1.find(3)->key(), 3);
  UF_TEST_EQUAL(n1.find(4)->key(), 4);
}

static void
test_tree_set_insert_unbalanced_reversed()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);

  UF_TEST(int_node_type::insert_unbalanced(&n4, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n1));

  UF_TEST_TREE_SET_NODE_HAS(&n4, &n3, 0,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n3, &n2, 0, &n4);
  UF_TEST_TREE_SET_NODE_HAS(&n2, &n1, 0, &n3);
  UF_TEST_TREE_SET_NODE_HAS(&n1,   0, 0, &n2);

  UF_TEST_EQUAL(n4.minimum(), &n1);
  UF_TEST_EQUAL(n1.next(), &n2);
  UF_TEST_EQUAL(n2.next(), &n3);
  UF_TEST_EQUAL(n3.next(), &n4);
  UF_TEST_EQUAL(n4.next(),   0);

  UF_TEST_EQUAL(n4.find(0), 0);
  UF_TEST_EQUAL(n4.find(5), 0);
  UF_TEST_EQUAL(n4.find(1)->key(), 1);
  UF_TEST_EQUAL(n4.find(2)->key(), 2);
  UF_TEST_EQUAL(n4.find(3)->key(), 3);
  UF_TEST_EQUAL(n4.find(4)->key(), 4);
}

static void
test_tree_set_find_fall_in_middle()
{
  int_node_type n1(1);
  int_node_type n3(3);
  int_node_type n5(5);

  UF_TEST(int_node_type::insert_unbalanced(&n3, &n1));
  UF_TEST(int_node_type::insert_unbalanced(&n3, &n5));

  UF_TEST_EQUAL(n3.find(0), 0);
  UF_TEST_EQUAL(n3.find(1)->key(), 1);
  UF_TEST_EQUAL(n3.find(2), 0);
  UF_TEST_EQUAL(n3.find(3)->key(), 3);
  UF_TEST_EQUAL(n3.find(4), 0);
  UF_TEST_EQUAL(n3.find(5)->key(), 5);
  UF_TEST_EQUAL(n3.find(6), 0);
}

static void
test_tree_set_insert_unbalanced_balanced()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);

  UF_TEST(int_node_type::insert_unbalanced(&n2, &n1));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n4));

  UF_TEST_TREE_SET_NODE_HAS(&n2, &n1, &n3,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n1,   0,   0, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n3,   0, &n4, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n4,   0,   0, &n3);

  UF_TEST_EQUAL(n2.minimum(), &n1);
  UF_TEST_EQUAL(n1.next(), &n2);
  UF_TEST_EQUAL(n2.next(), &n3);
  UF_TEST_EQUAL(n3.next(), &n4);
  UF_TEST_EQUAL(n4.next(),   0);

  UF_TEST_EQUAL(n2.find(0), 0);
  UF_TEST_EQUAL(n2.find(5), 0);
  UF_TEST_EQUAL(n2.find(1)->key(), 1);
  UF_TEST_EQUAL(n2.find(2)->key(), 2);
  UF_TEST_EQUAL(n2.find(3)->key(), 3);
  UF_TEST_EQUAL(n2.find(4)->key(), 4);

}

static void
test_tree_set_insert_unbalanced_samekey()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type nx(3);

  UF_TEST( int_node_type::insert_unbalanced(&n1, &n2));
  UF_TEST( int_node_type::insert_unbalanced(&n1, &n3));
  UF_TEST(!int_node_type::insert_unbalanced(&n1, &nx));

}

static void
test_tree_set_remove_unbalanced_empty_root()
{
  int_node_type::removal_unbalance_t unb;
  int_node_type n1(1);
  int_node_type::remove_unbalanced(&n1, &n1, &unb);
  UF_TEST_TREE_SET_NODE_HAS(&n1, 0, 0, 0);
  UF_TEST_EQUAL(unb.m_child, 0);
  UF_TEST_EQUAL(unb.m_up, 0);
  UF_TEST_EQUAL(n1.key(), 1);
}

static void
test_tree_set_remove_unbalanced_nochild()
{
  int_node_type n1(1); n1.set_black(); 
  int_node_type n2(2); n2.set_black(); 
  int_node_type n3(3); n3.set_black(); 
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n1));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n3));

  int_node_type::removal_unbalance_t unb;
  int_node_type::remove_unbalanced(&n2, &n3, &unb);

  UF_TEST_EQUAL(unb.m_child, 0);
  UF_TEST_EQUAL(unb.m_up, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n2, &n1,   0,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n3,   0,   0,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n1,   0,   0, &n2);
}

static void
test_tree_set_remove_unbalanced_left_child()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);
  int_node_type n5(5);

  UF_TEST(int_node_type::insert_unbalanced(&n4, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n1));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n5));

  UF_TEST_TREE_SET_NODE_HAS(&n4, &n3, &n5,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n3, &n2,   0, &n4);
  UF_TEST_TREE_SET_NODE_HAS(&n2, &n1,   0, &n3);
  UF_TEST_TREE_SET_NODE_HAS(&n1,   0,   0, &n2);

  int_node_type::removal_unbalance_t unb;
  int_node_type::remove_unbalanced(&n4, &n2, &unb);
  UF_TEST_EQUAL(unb.m_child, &n1);
  UF_TEST_EQUAL(unb.m_up, &n3);

  UF_TEST_TREE_SET_NODE_HAS(&n4, &n3, &n5,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n3, &n1,   0, &n4);
  UF_TEST_TREE_SET_NODE_HAS(&n2,   0,   0,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n1,   0,   0, &n3);

}

static void
test_tree_set_remove_unbalanced_right_child()
{
  int_node_type n1(1); n1.set_black();
  int_node_type n2(2); n2.set_black();
  int_node_type n3(3); n3.set_black();
  int_node_type n4(4); n4.set_black();
  int_node_type n5(5); n5.set_black();

  UF_TEST(int_node_type::insert_unbalanced(&n2, &n1));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n4));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n5));

  UF_TEST_TREE_SET_NODE_HAS(&n2, &n1, &n3,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n3,   0, &n4, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n4,   0, &n5, &n3);
  UF_TEST_TREE_SET_NODE_HAS(&n5,   0,   0, &n4);

  int_node_type::removal_unbalance_t unb;
  int_node_type::remove_unbalanced(&n2, &n4, &unb);

  UF_TEST_EQUAL(unb.m_child, &n5);
  UF_TEST_EQUAL(unb.m_up, &n3);
  UF_TEST_TREE_SET_NODE_HAS(&n2, &n1, &n3,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n3,   0, &n5, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n4,   0,   0,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n5,   0,   0, &n3);
  //UF_TEST_EQUAL(tobal, &n5);
}

static void
test_tree_set_remove_unbalanced_both_children()
{
  int_node_type n1(1); n1.set_black();
  int_node_type n2(2); n2.set_black();
  int_node_type n3(3); n3.set_black();
  int_node_type n4(4); n4.set_black();
  int_node_type n5(5); n5.set_black();
  int_node_type n6(6); n6.set_black();
  int_node_type n7(7); n7.set_black();

  UF_TEST(int_node_type::insert_unbalanced(&n2, &n1));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n5));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n4));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n6));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n7));

  UF_TEST_TREE_SET_NODE_HAS(&n3,   0, &n5, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n5, &n4, &n6, &n3);
  UF_TEST_TREE_SET_NODE_HAS(&n4,   0,   0, &n5);
  UF_TEST_TREE_SET_NODE_HAS(&n6,   0, &n7, &n5);
  UF_TEST_TREE_SET_NODE_HAS(&n7,   0,   0, &n6);
  
  int_node_type::removal_unbalance_t unb;
  int_node_type::remove_unbalanced(&n2, &n5, &unb);

  UF_TEST_EQUAL(unb.m_child, &n7);
  UF_TEST_EQUAL(unb.m_up, &n6);
  UF_TEST_TREE_SET_NODE_HAS(&n3,   0, &n6, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n5,   0,   0,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n4,   0,   0, &n6);
  UF_TEST_TREE_SET_NODE_HAS(&n6, &n4, &n7, &n3);
  UF_TEST_TREE_SET_NODE_HAS(&n7,   0,   0, &n6);
  //UF_TEST_EQUAL(tobal, &n7);
}

static void
test_tree_set_remove_unbalanced_noup()
{
  int_node_type n1(1); n1.set_black();
  int_node_type n2(2); n2.set_black();
  int_node_type n3(3); n3.set_black();
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n3));

  int_node_type::removal_unbalance_t unb;
  int_node_type::remove_unbalanced(&n1, &n1, &unb);

  UF_TEST_EQUAL(unb.m_child, &n2);
  UF_TEST_EQUAL(unb.m_up, 0);
  UF_TEST_TREE_SET_NODE_HAS(&n1,   0,   0,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n2,   0, &n3,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n3,   0,   0, &n2);
  //UF_TEST_EQUAL(tobal, &n2);
}

static void
test_tree_set_remove_unbalanced_noup_both_children()
{
  int_node_type n1(1); n1.set_black();
  int_node_type n2(2); n2.set_black();
  int_node_type n3(3); n3.set_black();
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n1));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n3));

  int_node_type::removal_unbalance_t unb;
  int_node_type::remove_unbalanced(&n2, &n2, &unb);
  UF_TEST_EQUAL(unb.m_child,  0);
  UF_TEST_EQUAL(unb.m_up, &n3);
  UF_TEST_TREE_SET_NODE_HAS(&n1,   0,   0, &n3);
  UF_TEST_TREE_SET_NODE_HAS(&n2,   0,   0,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n3, &n1,   0,   0);
  //UF_TEST_EQUAL(tobal, 0);
}

static void
test_tree_set_node_minimum_maximum()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);
  int_node_type n5(5);
  int_node_type n6(6);
  int_node_type n7(7);
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n1));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n6));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n5));
  UF_TEST(int_node_type::insert_unbalanced(&n4, &n7));

  UF_TEST_EQUAL(n4.minimum()->key(), 1);
  UF_TEST_EQUAL(n4.maximum()->key(), 7);
}

static void
test_tree_set_node_rorate_left()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);
  int_node_type n5(5);

  UF_TEST(int_node_type::insert_unbalanced(&n1, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n4));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n5));
  int_node_type::rotate_left(&n2);

  UF_TEST_TREE_SET_NODE_HAS(&n1,   0, &n4,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n2,   0, &n3, &n4);
  UF_TEST_TREE_SET_NODE_HAS(&n3,   0,   0, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n4, &n2, &n5, &n1);
  UF_TEST_TREE_SET_NODE_HAS(&n5,   0,   0, &n4);
}

static void
test_tree_set_node_rorate_left_no_left()
{
  int_node_type n1(1);
  int_node_type n2(2);
  //int_node_type n3(3);
  int_node_type n4(4);
  int_node_type n5(5);

  UF_TEST(int_node_type::insert_unbalanced(&n1, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n4));
  //int_node_type::insert_unbalanced(&n1, &n3);
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n5));
  int_node_type::rotate_left(&n2);

  UF_TEST_TREE_SET_NODE_HAS(&n1,   0, &n4,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n2,   0,   0, &n4);
  //UF_TEST_TREE_SET_NODE_HAS(&n3,   0,   0, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n4, &n2, &n5, &n1);
  UF_TEST_TREE_SET_NODE_HAS(&n5,   0,   0, &n4);
}

static void
test_tree_set_node_rorate_left_no_root()
{
  //int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);
  int_node_type n5(5);

  //int_node_type::insert_unbalanced(&n1, &n2);
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n4));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n2, &n5));
  int_node_type::rotate_left(&n2);

  //UF_TEST_TREE_SET_NODE_HAS(&n1,   0, &n4,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n2,   0, &n3, &n4);
  UF_TEST_TREE_SET_NODE_HAS(&n3,   0,   0, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n4, &n2, &n5,   0);
  UF_TEST_TREE_SET_NODE_HAS(&n5,   0,   0, &n4);
}

static void
test_tree_set_node_rorate_right()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);
  int_node_type n5(5);

  UF_TEST(int_node_type::insert_unbalanced(&n5, &n4));
  UF_TEST(int_node_type::insert_unbalanced(&n5, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n5, &n1));
  UF_TEST(int_node_type::insert_unbalanced(&n5, &n3));
  int_node_type::rotate_right(&n4);

  UF_TEST_TREE_SET_NODE_HAS(&n1,   0,   0, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n2, &n1, &n4, &n5);
  UF_TEST_TREE_SET_NODE_HAS(&n3,   0,   0, &n4);
  UF_TEST_TREE_SET_NODE_HAS(&n4, &n3,   0, &n2);
  UF_TEST_TREE_SET_NODE_HAS(&n5, &n2,   0,   0);
}

template<class Key>
class node_depth_t
{
public:
  typedef typename uf::red_black_t<Key> node_type; 

  explicit node_depth_t(node_type* root) : m_root(root) {}

  size_t operator()() const
  {
	return m_root ? walk(m_root, 1) : 0;
  }

  size_t walk(node_type* n, size_t depth) const
  {
	return uf::max_of((n->left()  ? walk(n->left(), depth+1) : depth),
					  (n->right() ? walk(n->right(), depth+1) : depth));
  }

private:
  node_type* m_root;
};

template<class Key>
class node_dump_t
{
public:
  typedef uf::red_black_t<Key> node_type; 

  explicit node_dump_t(node_type* root) : m_root(root) {}

  void operator()() const
  {
	std::cerr << "tree:" << std::endl;
	return m_root ? walk(m_root, 1) : 0;
  }

  void walk(node_type* n, size_t depth) const
  {
	if (n->left()) { walk(n->left(), depth+1); }
	std::cerr << std::string(depth, ' ').c_str() << n->key();
	std::cerr << " " << (n->red() ? "[r]" : "[b]");
	std::cerr << std::endl;
	//std::cerr << "(" << depth << ")" << n->key() << std::endl;
	if (n->right()) { walk(n->right(), depth+1); }
  }

private:
  node_type* m_root;
};

template<class Key>
class node_ordered_t
{
public:
  typedef typename uf::red_black_t<Key> node_type; 
	typedef typename node_type::comparator_type comparator_type;

  node_ordered_t(node_type* root) : m_root(root) {}

  bool operator()() const
  {
	node_type* here = m_root->minimum();
	while (here) {
	  node_type* last = here;
	  here = here->next();
	  if (here && (!comparator_type()(last->key(), here->key()))) {
		return false;
	  }
	}

	return true;
  }

private:
  node_type* m_root;
};

static void
test_node_depth()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);

  UF_TEST(int_node_type::insert_unbalanced(&n1, &n2));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n3));
  UF_TEST(int_node_type::insert_unbalanced(&n1, &n4));

  UF_TEST_EQUAL(4, node_depth_t<int>(&n1)());
  UF_TEST(node_ordered_t<int>(&n1)());

  n1.set_black();
  UF_ASSERT(!n1.invariant());
}


static void
test_tree_set_insert_ordered()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);
  int_node_type n5(5);
  int_node_type n6(6);
  int_node_type n7(7);

  n1.set_black(); // root node should be black

  int_node_type* newroot = n1.root();
  UF_TEST(int_node_type::insert(newroot, &n2, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n3, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n4, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n5, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n6, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n7, &newroot));
  UF_TEST_EQUAL(4, node_depth_t<int>(newroot)());
  UF_TEST(node_ordered_t<int>(newroot)());

}

static void
test_tree_set_insert_reversed()
{
  int_node_type n1(1);
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);
  int_node_type n5(5);
  int_node_type n6(6);
  int_node_type n7(7);

  n7.set_black(); // root node should be black

  int_node_type* newroot = n7.root();
  UF_TEST(int_node_type::insert(newroot, &n6, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n5, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n4, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n3, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n2, &newroot));
  UF_TEST(int_node_type::insert(newroot, &n1, &newroot));
  UF_TEST_EQUAL(4, node_depth_t<int>(newroot)());
  UF_TEST(node_ordered_t<int>(newroot)());
}

typedef std::vector<int_node_type*> int_node_list_type;

static void
delete_node_in_list(int_node_list_type& list)
{
  for (int_node_list_type::iterator i=list.begin(); i<list.end(); ++i) {
	delete *i;
  }
}

static size_t depth_limit_of_size(size_t sz)
{
  return 2*uf::to_size(log(static_cast<double>(sz))/log(2.0));
}

static int_node_type* 
tree_set_insert_from_list(const int_node_list_type& list)
{
  int_node_type* root = *(list.begin());
  root->set_black();

  for (int_node_list_type::const_iterator i=list.begin()+1; i<list.end(); ++i) {
	int_node_type::insert(root, *i, &root);
	UF_TEST(root->invariant());
  }

  return root;
}

static int_node_type* 
tree_set_remove_from_list(int_node_type* root, const int_node_list_type& list)
{
  for (int_node_list_type::const_iterator i=list.begin(); i<list.end(); ++i) {
	int_node_type::remove(root, *i, &root);
	UF_TEST(root->invariant());
  }

  return root;
}

static void
test_tree_set_insert_large_order()
{
  int_node_list_type node_list;
  for (int i=0; i<128; ++i) { node_list.push_back(new int_node_type(i+1)); }
  int_node_type* root = tree_set_insert_from_list(node_list);

  UF_ASSERT(node_depth_t<int>(root)() <= depth_limit_of_size(node_list.size()));
  UF_ASSERT(root->invariant());
  UF_TEST_EQUAL(node_list.size(), root->size());

  delete_node_in_list(node_list);
}

static void
test_tree_set_insert_large_reverse()
{
  int_node_list_type node_list;
  for (int i=0; i<128; ++i) { node_list.push_back(new int_node_type(i+1)); }
  std::reverse(node_list.begin(), node_list.end());
  int_node_type* root = tree_set_insert_from_list(node_list);

  UF_ASSERT(node_depth_t<int>(root)() <= depth_limit_of_size(node_list.size()));
  UF_ASSERT(root->invariant());
  UF_TEST_EQUAL(node_list.size(), root->size());

  delete_node_in_list(node_list);
}

static void
test_tree_set_insert_large_shuffle()
{
  int_node_list_type node_list;
  for (int i=0; i<128; ++i) { node_list.push_back(new int_node_type(i+1)); }
  std::random_shuffle(node_list.begin(), node_list.end());
  int_node_type* root = tree_set_insert_from_list(node_list);

  UF_ASSERT(node_depth_t<int>(root)() <= depth_limit_of_size(node_list.size()));
  UF_ASSERT(root->invariant());
  UF_TEST_EQUAL(node_list.size(), root->size());

  delete_node_in_list(node_list);
}

static void
test_tree_set_remove_large_first_half_order()
{
  int_node_list_type node_list;
  for (int i=0; i<128; ++i) { node_list.push_back(new int_node_type(i+1)); }
  int_node_type* root = tree_set_insert_from_list(node_list);
  int_node_list_type toremove(node_list.begin(), node_list.begin() + node_list.size()/2);
  root = tree_set_remove_from_list(root, toremove);

  //{ node_dump_t<int> d(root); d(); }
  UF_TEST(node_depth_t<int>(root)() <= depth_limit_of_size(node_list.size() - toremove.size()));
  UF_TEST(root->invariant());
  //{ node_dump_t<int> d(root); d(); }
  UF_TEST_EQUAL(node_list.size() - toremove.size(), root->size());

  delete_node_in_list(node_list);
}

static void
test_tree_set_remove_large_last_half_order()
{
  int_node_list_type node_list;
  for (int i=0; i<128; ++i) { node_list.push_back(new int_node_type(i+1)); }
  int_node_type* root = tree_set_insert_from_list(node_list);

  int_node_list_type toremove(node_list.begin() + node_list.size()/2, node_list.end());
  root = tree_set_remove_from_list(root, toremove);
  
  UF_TEST(node_depth_t<int>(root)() <= depth_limit_of_size(node_list.size() - toremove.size()));
  UF_TEST(root->invariant());
  UF_TEST_EQUAL(node_list.size() - toremove.size(), root->size());

  delete_node_in_list(node_list);
}

static void
test_tree_set_remove_large_middle_half_order()
{
  int_node_list_type node_list;
  for (int i=0; i<128; ++i) { node_list.push_back(new int_node_type(i+1)); }
  int_node_type* root = tree_set_insert_from_list(node_list);

  int_node_list_type toremove(node_list.begin() + (node_list.size()*1)/4, 
							  node_list.begin() + (node_list.size()*3)/4);
  root = tree_set_remove_from_list(root, toremove);
  
  UF_TEST(node_depth_t<int>(root)() <= depth_limit_of_size(node_list.size() - toremove.size()));
  UF_TEST(root->invariant());
  UF_TEST_EQUAL(node_list.size() - toremove.size(), root->size());

  delete_node_in_list(node_list);
}

static void
test_tree_set_remove_large_first_half_reversed()
{
  int_node_list_type node_list;
  for (int i=0; i<128; ++i) { node_list.push_back(new int_node_type(i+1)); }
  int_node_type* root = tree_set_insert_from_list(node_list);
  int_node_list_type toremove(node_list.begin(), node_list.begin() + node_list.size()/2);
  root = tree_set_remove_from_list(root, toremove);
  std::reverse(toremove.begin(), toremove.end());

  UF_TEST(node_depth_t<int>(root)() <= depth_limit_of_size(node_list.size() - toremove.size()));
  UF_TEST(root->invariant());
  UF_TEST_EQUAL(node_list.size() - toremove.size(), root->size());

  delete_node_in_list(node_list);

}

static void
test_tree_set_remove_large_first_half_shuffle()
{
  int_node_list_type node_list;
  for (int i=0; i<128; ++i) { node_list.push_back(new int_node_type(i+1)); }

  std::random_shuffle(node_list.begin(), node_list.end());
  int_node_type* root = tree_set_insert_from_list(node_list);

  int_node_list_type toremove(node_list.begin(), node_list.begin() + node_list.size()/2);
  std::random_shuffle(toremove.begin(), toremove.end());

  root = tree_set_remove_from_list(root, toremove);
  UF_TEST(node_depth_t<int>(root)() <= depth_limit_of_size(node_list.size() - toremove.size()));
  UF_TEST(root->invariant());
  UF_TEST_EQUAL(node_list.size() - toremove.size(), root->size());

  delete_node_in_list(node_list);

}

static void
test_tree_set_remove_large_last_half_reversed()
{
  int_node_list_type node_list;
  for (int i=0; i<128; ++i) { node_list.push_back(new int_node_type(i+1)); }
  int_node_type* root = tree_set_insert_from_list(node_list);
  int_node_list_type toremove(node_list.begin() + node_list.size()/2, node_list.end());
  std::reverse(toremove.begin(), toremove.end());
  root = tree_set_remove_from_list(root, toremove);

  UF_TEST(node_depth_t<int>(root)() <= depth_limit_of_size(node_list.size() - toremove.size()));
  UF_TEST(root->invariant());
  UF_TEST_EQUAL(node_list.size() - toremove.size(), root->size());

  delete_node_in_list(node_list);
}

static void
test_tree_set_dfs_iterator_full()
{
  int_node_type n1(1); 
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);
  int_node_type n5(5);
  int_node_type n6(6);
  int_node_type n7(7);

  n4.set_black(); // root node should be black
  int_node_type* root = n4.root();
  UF_TEST(int_node_type::insert_unbalanced(root, &n2));
  UF_TEST(int_node_type::insert_unbalanced(root, &n6));
  UF_TEST(int_node_type::insert_unbalanced(root, &n1));
  UF_TEST(int_node_type::insert_unbalanced(root, &n3));
  UF_TEST(int_node_type::insert_unbalanced(root, &n5));
  UF_TEST(int_node_type::insert_unbalanced(root, &n7));

  uf::red_black_dfs_iterator_t<int> i(&n4);
  UF_TEST_EQUAL(*i, 1);
  i++;
  UF_TEST_EQUAL(*i, 3);
  i++;
  UF_TEST_EQUAL(*i, 2);
  i++;
  UF_TEST_EQUAL(*i, 5);
  i++;
  UF_TEST_EQUAL(*i, 7);
  i++;
  UF_TEST_EQUAL(*i, 6);
  i++;
  UF_TEST_EQUAL(*i, 4);
  i++;
  UF_TEST(i.atend());
}

static void
test_tree_set_dfs_iterator_right()
{
  int_node_type n1(1); 
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);

  n4.set_black(); // root node should be black
  int_node_type* root = n1.root();
  UF_TEST(int_node_type::insert_unbalanced(root, &n2));
  UF_TEST(int_node_type::insert_unbalanced(root, &n3));
  UF_TEST(int_node_type::insert_unbalanced(root, &n4));

  uf::red_black_dfs_iterator_t<int> i(&n1);
  UF_TEST_EQUAL(*i, 4);
  i++;
  UF_TEST_EQUAL(*i, 3);
  i++;
  UF_TEST_EQUAL(*i, 2);
  i++;
  UF_TEST_EQUAL(*i, 1);
  i++;
  UF_TEST(i.atend());
}

static void
test_tree_set_dfs_iterator_left()
{
  int_node_type n1(1); 
  int_node_type n2(2);
  int_node_type n3(3);
  int_node_type n4(4);

  n4.set_black(); // root node should be black
  int_node_type* root = n4.root();
  UF_TEST(int_node_type::insert_unbalanced(root, &n3));
  UF_TEST(int_node_type::insert_unbalanced(root, &n2));
  UF_TEST(int_node_type::insert_unbalanced(root, &n1));

  uf::red_black_dfs_iterator_t<int> i(&n4);
  UF_TEST_EQUAL(*i, 1);
  i++;
  UF_TEST_EQUAL(*i, 2);
  i++;
  UF_TEST_EQUAL(*i, 3);
  i++;
  UF_TEST_EQUAL(*i, 4);
  i++;
  UF_TEST(i.atend());
}

void test_red_black()
{
  test_tree_set_empty_node();
  test_tree_set_next_prior();
  test_tree_set_insert_unbalanced_ordered();
  test_tree_set_insert_unbalanced_reversed();
  test_tree_set_insert_unbalanced_balanced();
  test_tree_set_insert_unbalanced_samekey();
  test_tree_set_find_fall_in_middle();
  test_tree_set_remove_unbalanced_empty_root();
  test_tree_set_remove_unbalanced_nochild();
  test_tree_set_remove_unbalanced_left_child();
  test_tree_set_remove_unbalanced_right_child();
  test_tree_set_remove_unbalanced_both_children();
  test_tree_set_remove_unbalanced_noup();
  test_tree_set_remove_unbalanced_noup_both_children();
  test_tree_set_node_minimum_maximum();
  test_tree_set_node_rorate_left();
  test_tree_set_node_rorate_left_no_left();
  test_tree_set_node_rorate_left_no_root();
  test_tree_set_node_rorate_right();
  test_node_depth();
  test_tree_set_insert_ordered();
  test_tree_set_insert_reversed();
  test_tree_set_insert_large_order();
  test_tree_set_insert_large_reverse();
  test_tree_set_insert_large_shuffle();
  test_tree_set_remove_large_first_half_order();
  test_tree_set_remove_large_last_half_order();
  test_tree_set_remove_large_middle_half_order();
  test_tree_set_remove_large_first_half_reversed();
  test_tree_set_remove_large_last_half_reversed();
  test_tree_set_remove_large_first_half_shuffle();
  test_tree_set_dfs_iterator_full();
  test_tree_set_dfs_iterator_right();
  test_tree_set_dfs_iterator_left();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
