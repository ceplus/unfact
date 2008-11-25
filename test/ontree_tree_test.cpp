
#include <ontree/zone.hpp>
#include <ontree/tree.hpp>
#include <string>

using namespace ontree;

void test_zone_hello()
{
  zone_t z;
  byte_t* p0 = z.allocate(10);
  byte_t* p1 = z.allocate(20);
  byte_t* p2 = z.allocate(20);
  // we test nothing here... should check leakage.
}

void test_zone_large_block()
{
  zone_t z(128);
  byte_t* small  = z.allocate(64);
  byte_t* medium = z.allocate(100);
  byte_t* large  = z.allocate(200);
};

void test_tree_hello()
{
  tree_t t;
  object_t* o = t.root();

  number_t* n = t.insert_leaf(o, "hoge", 1.0f);
  string_t* s = t.insert_leaf(o, "hoge", "hello");
  predicate_t* p = t.insert_leaf(o, "fuga", true);
  null_t* nl = t.insert_leaf(o, "ika", none_t());
  object_t* c = t.insert_object(o, "tako");

  assert(0 == o->parent());
  assert(1.0 == n->value());
  assert(o == n->parent());
  assert("hello" == s->value());
  assert(o == s->parent());
  assert(true == p->value());
  assert(o == p->parent());
  assert(type_null == nl->type());
  assert(o == nl->parent());
  assert(type_object == c->type());
  assert(o == c->parent());

  array_t* a = t.insert_array(o, "array");

  number_t* an = t.insert_leaf(a, 2.0f);
  string_t* as = t.insert_leaf(a, "bye");
  predicate_t* ap = t.insert_leaf(a, false);
  null_t* anl = t.insert_leaf(a, none_t());
  object_t* ac = t.insert_object(a);
  array_t* aa = t.insert_array(a);

  assert(o == a->parent());
  assert(2.0 == an->value());
  assert(a == an->parent());
  assert("bye" == as->value());
  assert(a == as->parent());
  assert(false == ap->value());
  assert(a == ap->parent());
  assert(type_null == anl->type());
  assert(a == anl->parent());
  assert(type_object == ac->type());
  assert(a == ac->parent());
  assert(type_array  == aa->type());
  assert(a == aa->parent());

}

void test_tree_object_iterator()
{
  tree_t t;
  object_t* o = t.root();

  t.insert_leaf(o, "foo", 1.0f);
  t.insert_leaf(o, "bar", 2.0f);
  t.insert_leaf(o, "baz", 3.0f);

  assert(o->size() == 3);

  member_iterator_t beg = o->begin();
  member_iterator_t end = o->end();
  member_iterator_t i = beg;

  assert(beg.child() == member_t::unnode(beg.node()));

  assert(i != end);
  assert(i->type() == type_number);
  assert(child_cast<number_t>(i)->value() == 1.0f);

  ++i;
  assert(i != end);
  assert(i->type() == type_number);
  assert(child_cast<number_t>(i)->value() == 2.0f);

  ++i;
  assert(i != end);
  assert(i->type() == type_number);
  assert(child_cast<number_t>(i)->value() == 3.0f);

  ++i;
  assert(i == end);
	
  member_iterator_t ifoo = o->find("foo");
  assert(ifoo.name() == "foo");
  member_iterator_t ihoge = o->find("hoge");
  assert(ihoge == o->end());
	
}

void test_tree_array_iterator()
{
  tree_t t;
  object_t* o = t.root();
  array_t* a = t.insert_array(o, "array");

  t.insert_leaf(a, 1.0f);
  t.insert_leaf(a, 2.0f);
  t.insert_leaf(a, 3.0f);

  assert(a->size() == 3);

  element_iterator_t beg = a->begin();
  element_iterator_t end = a->end();
  element_iterator_t i = beg;

  assert(beg.child() == element_t::unnode(beg.node()));

  assert(i != end);
  assert(i->type() == type_number);
  assert(child_cast<number_t>(i)->value() == 1.0f);

  ++i;
  assert(i != end);
  assert(i->type() == type_number);
  assert(child_cast<number_t>(i)->value() == 2.0f);

  ++i;
  assert(i != end);
  assert(i->type() == type_number);
  assert(child_cast<number_t>(i)->value() == 3.0f);

  ++i;
  assert(i == end);

  i = a->at(1);
  assert(i->type() == type_number);
  assert(child_cast<number_t>(i)->value() == 2.0f);

  i = a->at(3);
  assert(i == a->end());
}

void test_tree_copy_with_text()
{
  tree_t* t0 = new tree_t();

  t0->insert_leaf(t0->root(), "foo", "bar");
  tree_t t1(*t0);

  delete t0; // to release internal storage

  assert("foo" == t1.root()->begin().name());
  assert("bar" == std::string(child_cast<string_t>(t1.root()->begin())->value().c_str()));
}

void test_tree()
{
  test_zone_hello();
  test_zone_large_block();
  test_tree_hello();
  test_tree_object_iterator();
  test_tree_array_iterator();
  test_tree_copy_with_text();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
