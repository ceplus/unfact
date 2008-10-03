
#include <ontree/writer.hpp>
#include <vector>
#include <string>

using namespace ontree;

struct writer_tester_t
{
  writer_tester_t(const char* expected)
	: m_expected(expected), 
	  m_buffer(m_expected.size()*2, '\0')
  {
	m_writer.set_buffer(&(m_buffer[0]), m_buffer.size());
	m_head = &(m_buffer[0]);
  }

  bool matched() const
  {
	return (m_expected == std::string(&(m_buffer[0])));
  }

  writer_t& writer() { return m_writer; }

  std::string m_expected;
  std::vector<char> m_buffer;
  writer_t m_writer;
  const char* m_head; // for debugger inspection
};

void test_writer_hello()
{
  writer_tester_t t0("{}");
  assert(is_ok(t0.writer().write_begin(scope_object)));
  assert(is_ok(t0.writer().write_end()));
  assert(t0.matched());

  /* trivial flat object */
  writer_tester_t t1("{\"foo\":\"bar\",\"hoge\":\"ika\"}");
  assert(is_ok(t1.writer().write_begin(scope_object)));
  assert(is_ok(t1.writer().write_key("foo")));
  assert(is_ok(t1.writer().write_value("bar")));
  assert(is_ok(t1.writer().write_key("hoge")));
  assert(is_ok(t1.writer().write_value("ika")));
  assert(is_ok(t1.writer().write_end()));
  assert(t1.matched());

  /* array */
  writer_tester_t t2("{\"foo\":[\"hoge\",\"ika\"]}");
  assert(is_ok(t2.writer().write_begin(scope_object)));
  assert(is_ok(t2.writer().write_key("foo")));
  assert(is_ok(t2.writer().write_begin(scope_array)));
  assert(is_ok(t2.writer().write_value("hoge")));
  assert(is_ok(t2.writer().write_value("ika")));
  assert(is_ok(t2.writer().write_end()));
  assert(is_ok(t2.writer().write_end()));
  assert(t2.matched());

  /* nested object */
  writer_tester_t t3("{\"foo\":{\"hoge\":\"ika\"}}");
  assert(is_ok(t3.writer().write_begin(scope_object)));
  assert(is_ok(t3.writer().write_key("foo")));
  assert(is_ok(t3.writer().write_begin(scope_object)));
  assert(is_ok(t3.writer().write_key("hoge")));
  assert(is_ok(t3.writer().write_value("ika")));
  assert(is_ok(t3.writer().write_end()));
  assert(is_ok(t3.writer().write_end()));
  assert(t3.matched());

  /* number */
  writer_tester_t t4("{\"foo\":1.5}");
  assert(is_ok(t4.writer().write_begin(scope_object)));
  assert(is_ok(t4.writer().write_key("foo")));
  assert(is_ok(t4.writer().write_value(1.5f)));
  assert(is_ok(t4.writer().write_end()));
  assert(t4.matched());

  /* predicate */
  writer_tester_t t5("{\"foo\":true}");
  assert(is_ok(t5.writer().write_begin(scope_object)));
  assert(is_ok(t5.writer().write_key("foo")));
  assert(is_ok(t5.writer().write_value(true)));
  assert(is_ok(t5.writer().write_end()));
  assert(t5.matched());

  /* predicate */
  writer_tester_t t6("{\"foo\":null}");
  assert(is_ok(t6.writer().write_begin(scope_object)));
  assert(is_ok(t6.writer().write_key("foo")));
  assert(is_ok(t6.writer().write_value(none_t())));
  assert(is_ok(t6.writer().write_end()));
  assert(t6.matched());

}

void test_obufstream_hello()
{
  char buf[256];
  obufstream_t out(buf, 256);
  out << 1;
  out.terminate_cstr();
  assert(0 == strncmp(buf, "1", 256));
}

void test_writer()
{
  test_writer_hello();
  test_obufstream_hello();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
