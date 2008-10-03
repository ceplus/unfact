
#include <ontree/builder.hpp>

using namespace ontree;

struct builder_tester_t
{
  builder_tester_t(const char* message)
  {
	error_e err = build_tree(&m_tree, message);
	assert(is_ok(err));
  }

  tree_t& tree() { return m_tree; }
  object_t* root() const { return m_tree.root(); }

  tree_t m_tree;
};

void test_builder_hello()
{
  builder_tester_t bt0("{}");
  assert(0 == bt0.root()->size());

  builder_tester_t bt1("{\"foo\": 1.0, \"bar\": \"escaped\\nstring\\n\"}");
  assert(2 == bt1.root()->size());
  assert(bt1.root()->find("foo")->type() == type_number);
  assert(child_cast<number_t>(bt1.root()->find("foo"))->value() == 1.0);
  assert(child_cast<string_t>(bt1.root()->find("bar"))->value() == "escaped\nstring\n");

  builder_tester_t bt2("{\"foo\": true}");
  assert(1 == bt2.root()->size());
  assert(child_cast<predicate_t>(bt2.root()->find("foo"))->value() == true);

  builder_tester_t bt3("{\"foo\": null}");
  assert(child_cast<null_t>(bt3.root()->find("foo")));

  builder_tester_t bt4("{\"foo\": [0, 1]}");
  assert(bt4.root()->find("foo")->type() == type_array);
  assert(child_cast<array_t>(bt4.root()->find("foo"))->size() == 2);

  builder_tester_t bt5("{\"foo\": {\"hoge\": 0, \"ika\": 1}}");
  assert(bt5.root()->find("foo")->type() == type_object);
  assert(child_cast<object_t>(bt5.root()->find("foo"))->size() == 2);
}

void test_tree_copy()
{
  builder_tester_t bt("{\"foo\": {\"hoge\": 0, \"ika\": [1, 2, {\"foo\":\"bar\"}]}}");
  tree_t t2(bt.tree());
  assert(t2.size() == bt.tree().size());

  tree_t t3;
  t3.insert_leaf(t3.root(), "hoge", 1.0f);
  t2 = t3;
  assert(t2.size() == t3.size());
}

void test_builder()
{
  test_builder_hello();
  test_tree_copy();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
