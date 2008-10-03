
#include <ontree/generator.hpp>
#include <ontree/builder.hpp>
#include <vector>
#include <string>

using namespace ontree;

struct generator_tester_t
{
  generator_tester_t(const char* src)
	: m_generator(&m_tree)
  {
	error_e err = build_tree(&m_tree, src);
	assert(is_ok(err));
  }

  bool match_next(event_e evt)
  {
	m_generator.next();
	return evt == m_generator.last();
  }

  bool match(event_e evt)
  {
	return evt == m_generator.last();
  }
	
  tree_t m_tree;
  generator_t m_generator;
};

static bool match_regenerated(const std::string& str)
{
  error_e err = error_ok;
  tree_t tree;
  err = build_tree(&tree, str.c_str());
  assert(is_ok(err));
  std::string regen = generate_string(tree, 1);
  return str == regen;
}

void test_generator_hello()
{
  {
	generator_tester_t t("{}");
	assert(!t.m_generator.done());
	assert(t.match(event_begin));
	assert(t.match_next(event_object_begin));
	assert(t.match_next(event_object_end));
	assert(t.match_next(event_end));
	assert(t.m_generator.done());
  }

  {
	generator_tester_t t("{\"foo\":\"bar\"}");
	assert(!t.m_generator.done());
	assert(t.match(event_begin));
	assert(t.match_next(event_object_begin));
	assert(t.match_next(event_object_key));
	assert(t.match_next(event_string));
	assert(t.match_next(event_object_end));
	assert(t.match_next(event_end));
	assert(t.m_generator.done());
  }

  {
	generator_tester_t t("{\"foo\":[]}");
	assert(!t.m_generator.done());
	assert(t.match(event_begin));
	assert(t.match_next(event_object_begin));
	assert(t.match_next(event_object_key));
	assert(t.match_next(event_array_begin));
	assert(t.match_next(event_array_end));
	assert(t.match_next(event_object_end));
	assert(t.match_next(event_end));
	assert(t.m_generator.done());
  }

  {
	generator_tester_t t("{\"foo\":[1.0, \"bar\", true, null]}");
	assert(!t.m_generator.done());
	assert(t.match(event_begin));
	assert(t.match_next(event_object_begin));
	assert(t.match_next(event_object_key));
	assert(t.match_next(event_array_begin));
	assert(t.match_next(event_number));
	assert(t.match_next(event_string));
	assert(t.match_next(event_predicate));
	assert(t.match_next(event_null));
	assert(t.match_next(event_array_end));
	assert(t.match_next(event_object_end));
	assert(t.match_next(event_end));
	assert(t.m_generator.done());
  }

  {
	generator_tester_t t("{\"foo\":{\"num\":1.0, \"str\":\"bar\", \"pred\":true, \"null\":null}}");
	assert(!t.m_generator.done());
	assert(t.match(event_begin));
	assert(t.match_next(event_object_begin));
	assert(t.match_next(event_object_key));
	assert(std::string("foo") == t.m_generator.key());
	assert(t.match_next(event_object_begin));
	assert(t.match_next(event_object_key));
	assert(t.match_next(event_number));
	assert(1.0 == t.m_generator.number());
	assert(t.match_next(event_object_key));
	assert(t.match_next(event_string));
	assert(std::string("bar") == t.m_generator.string());
	assert(t.match_next(event_object_key));
	assert(t.match_next(event_predicate));
	assert(true == t.m_generator.predicate());
	assert(t.match_next(event_object_key));
	assert(std::string("null") == t.m_generator.key());
	assert(t.match_next(event_null));
	assert(t.match_next(event_object_end));
	assert(t.match_next(event_object_end));
	assert(t.match_next(event_end));
	assert(t.m_generator.done());
  }
}

void test_generator_regen()
{
  assert(match_regenerated("{}"));
  assert(match_regenerated("{\"foo\":\"bar\"}"));
  assert(match_regenerated("{\"foo\":[]}"));
  assert(match_regenerated("{\"foo\":[1.5,\"bar\",true,null]}"));
  assert(match_regenerated("{\"foo\":{\"num\":1.5,\"str\":\"bar\",\"pred\":true,\"null\":null}}"));
}

void test_generator()
{
  test_generator_hello();
  test_generator_regen();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
