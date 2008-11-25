
#include <ontree/find.hpp>
#include <ontree/builder.hpp>

using namespace ontree;

class test_tree_t : public tree_t
{
public:
  test_tree_t(const char* on)
  {
	error_e err = build_tree(this, on);
	assert(is_ok(err));
  }

  node_t* find(const char* path)
  {
	finder_t f(root());
	node_t* ret = 0;
	error_e err = f.find(path, &ret);
	assert(is_ok(err));
	return ret;
  }

  error_e find_error(const char* path)
  {
	finder_t f(root());
	node_t* found = 0;
	return f.find(path, &found);
  }

};

void test_find_hello()
{
  test_tree_t t0("{\"foo\":{\"str\":\"hello\",\"num\":1.5,\"pred\":true,\"null\":null}}");
  node_t* n0a = t0.find("foo.str");
  assert(n0a->type() == type_string);
  node_t* n0b = t0.find("foo.notfound");
  assert(0 == n0b);
  assert(error_not_found == t0.find_error("foo[0]")); // indexed access to an object is invalid
  assert(error_not_found == t0.find_error("foo.bar.baz")); // name access to non-object is invalid

  test_tree_t t1("{\"foo\":[\"hello\",1.5,true,null]}");
  node_t* n1a = t1.find("foo[0]");
  assert(n1a->type() == type_string);
  node_t* n1b = t1.find("foo[1]");
  assert(n1b->type() == type_number);
  node_t* n1c = t1.find("foo[5]");
  assert(0 == n1c);

  assert(error_not_found == t1.find_error("foo.bar")); // name access to array is invalid
  assert(error_not_found == t1.find_error("foo.0")); // dot with number is invalid
  assert(error_ill_formed == t1.find_error("foo[hoge]")); // does not support string 
  assert(error_ill_formed == t1.find_error("foo[-1]")); // does not support negative number



}

void test_fidner_read_path_hello()
{
  finder_t::path_t p0 = finder_t::read_path("foo");
  assert(p0.type == type_object);
  assert(std::string(p0.u.name.str, p0.u.name.size) == "foo");
  assert(p0.left.str() == "");

  finder_t::path_t p1 = finder_t::read_path("foo.bar");
  assert(p1.type == type_object);
  assert(std::string(p1.u.name.str, p1.u.name.size) == "foo");
  assert(p1.left.str() == ".bar");

  finder_t::path_t p2 = finder_t::read_path(".bar");
  assert(p2.type == type_object);
  assert(std::string(p2.u.name.str, p2.u.name.size) == "bar");
  assert(p2.left.str() == "");

  finder_t::path_t p3 = finder_t::read_path("[2].bar");
  assert(p3.type == type_array);
  assert(p3.u.index == 2);
  assert(p3.left.str() == ".bar");

  // should accept "_"
  finder_t::path_t p4 = finder_t::read_path("foo_bar");
  assert(p4.type == type_object);
  assert(std::string(p4.u.name.str, p4.u.name.size) == "foo_bar");
  assert(p4.left.str() == "");

}

void test_find()
{
  test_find_hello();
  test_fidner_read_path_hello();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
