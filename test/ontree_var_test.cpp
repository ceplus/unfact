
#include <ontree/var.hpp>
#include <ontree/builder.hpp>

using namespace ontree;

struct var_tester_t
{
public:
  explicit var_tester_t(const char* str)
  {
	error_e err = build_tree(&m_tree, str);
	assert(is_ok(err));
  }
	
  tree_t& tree() { return m_tree; }
  node_t* root() { return m_tree.root(); }

private:
  tree_t m_tree;
};

void test_var_hello()
{
  var_tester_t t("{\"foo\":{\"str\":\"hello\",\"num\":1.5,\"pred\":true,\"null\":null}}");
  var_t v = t.root();
  assert( v["foo"].defined());
  assert(!v["unfoo"].defined());
  assert(4 == v["foo"].size());
  assert(v["foo"]["str"].to_s() == "hello");
  assert(v["foo"]["str"].name() == "str");
  assert(v["foo"]["num"].to_f() == 1.5f);
  assert(v["foo"]["pred"].to_p() == true);
  assert(v["foo"]["null"].null());
}

void test_var_array()
{
  var_tester_t t("{\"foo\":[\"hello\",1.5,true,null],\"bar\":[]}");
  var_t v = t.root();
  assert( v["foo"].defined());
  assert(v["foo"][size_t(0)].to_s() == "hello");
  assert(v["foo"][1].to_f() == 1.5f);
  assert(v["foo"][2].to_p() == true);
  assert(v["foo"][3].null());
  assert(!v["foo"][4].defined());
  assert(4 == v["foo"].size());
  assert(0 == v["bar"].size());

  assert(v.find("foo[0]").type() == type_string);

  assert(!v["bar"].first().defined());

  var_t foo_child = v["foo"].first();
  assert(foo_child.type() == type_string);
  foo_child++;
  assert(foo_child.type() == type_number);
  foo_child++;
  assert(foo_child.type() == type_predicate);
  foo_child++;
  assert(foo_child.type() == type_null);
  foo_child++;
  assert(foo_child.type() == type_undefined);
}


void test_var()
{
  test_var_hello();
  test_var_array();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
