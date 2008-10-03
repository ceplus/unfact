
#include <ontree/reader.hpp>

using namespace ontree;

void test_reader_object_begin(const char* str, size_t left)
{
  reader_t r0;
  r0.set_buffer(str);
  assert(is_ok(r0.read()));
  assert(event_object_begin == r0.last());
  assert(r0.buffer().size() == left);
  assert(1 == r0.scope_depth());
}

void test_reader_object_begin_end(const char* str, size_t left)
{
  reader_t r0;
  r0.set_buffer(str);
  assert(is_ok(r0.read()));
  assert(event_object_begin == r0.last());
  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(r0.buffer().size() == left);
  assert(0 == r0.scope_depth());
}

void test_reader_object_key(const char* str, const char* keyval, size_t left)
{
  reader_t r0;
  r0.set_buffer(str);
  r0.set_last(event_object_begin);
  assert(is_ok(r0.read()));
  assert(event_object_key == r0.last());
  assert(r0.raw_string() == const_range_t(keyval));
  assert(r0.buffer().size() == left);
}

void test_reader_hello()
{
  test_reader_object_begin(" { ", 0);
  test_reader_object_begin(" {", 0);
  test_reader_object_begin("{ ", 0);
  test_reader_object_begin("{", 0);
  test_reader_object_begin("{ hoge", 4);

  test_reader_object_begin_end(" { }", 0);
  test_reader_object_begin_end(" {} ", 0);
  test_reader_object_begin_end(" {} ,", 1);

  test_reader_object_key("\"foo\":", "foo", 0);
  test_reader_object_key("\"foo\": ", "foo", 0);
  test_reader_object_key(" \"foo\": ", "foo", 0);
  test_reader_object_key(" \"foo\":", "foo", 0);
  test_reader_object_key(" \"foo\": 10", "foo", 2);
  test_reader_object_key(" \"foo\\tbar\":", "foo\\tbar", 0);
  test_reader_object_key(" \"foo\\u1234bar\":", "foo\\u1234bar", 0);
}

void test_const_range_eq()
{
  const_range_t r0a("hoge");
  const_range_t r0b("hoge");
  const_range_t r1("fuga");
  assert(r0a == r0b);
  assert(r0a != r1);
}

void test_helper_read_until_key(reader_t* reader, const char* keyname)
{
  assert(is_ok(reader->read()));
  assert(event_object_begin == reader->last());
  assert(is_ok(reader->read()));
  assert(event_object_key == reader->last());
  assert(reader->raw_string() == const_range_t(keyname));
}

void test_reader_object_value_string()
{
  reader_t r0;
  r0.set_buffer("{\"foo\": \"bar\"}");

  test_helper_read_until_key(&r0, "foo");

  assert(is_ok(r0.read()));
  assert(event_string == r0.last());
  assert(r0.raw_string() == const_range_t("bar"));
  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(r0.buffer().size() == 0);
}

void test_reader_object_value_string_mb()
{
  // {"message": "‚±‚ñ‚É‚¿‚Í"} in utf-8
  char data[] = { char(0x7b), char(0x22), char(0x6d), char(0x65), char(0x73), char(0x73), char(0x61), char(0x67), char(0x65), char(0x22), char(0x3a), char(0x20), char(0x22), char(0xe3), char(0x81), char(0x93), char(0xe3), char(0x82), char(0x93), char(0xe3), char(0x81), char(0xab), char(0xe3), char(0x81), char(0xa1), char(0xe3), char(0x81), char(0xaf), char(0x22), char(0x7d), char(0x00) };

  reader_t r0;
  r0.set_buffer(data);

  assert(is_ok(r0.read()));
  assert(event_object_begin == r0.last());
  assert(is_ok(r0.read()));
  assert(event_object_key == r0.last());
  assert(r0.raw_string() == const_range_t("message"));
  assert(is_ok(r0.read()));
  assert(event_string == r0.last());
  // assert(r0.raw_string() == const_range_t("bar"));
  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(r0.buffer().size() == 0);

}

void test_reader_object_value_number()
{
  reader_t r0;
  r0.set_buffer("{\"foo\": 10.5}");

  test_helper_read_until_key(&r0, "foo");

  assert(is_ok(r0.read()));
  assert(event_number == r0.last());
  assert(r0.number() == 10.5f);
  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(r0.buffer().size() == 0);
}

void test_reader_object_value_null()
{
  reader_t r0;
  r0.set_buffer("{\"foo\": null}");

  test_helper_read_until_key(&r0, "foo");

  assert(is_ok(r0.read()));
  assert(event_null == r0.last());
  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(r0.buffer().size() == 0);
}

void test_reader_object_value_object()
{
  reader_t r0;
  r0.set_buffer("{\"foo\": {\"bar\": \"baz\"}}");

  assert(is_ok(r0.read()));
  assert(event_object_begin == r0.last());
  assert(1 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_object_key == r0.last());
  assert(r0.raw_string() == const_range_t("foo"));

  assert(is_ok(r0.read()));
  assert(event_object_begin == r0.last());
  assert(2 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_object_key == r0.last());
  assert(r0.raw_string() == const_range_t("bar"));

  assert(is_ok(r0.read()));
  assert(event_string == r0.last());
  assert(r0.raw_string() == const_range_t("baz"));

  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(1 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(0 == r0.scope_depth());

  assert(r0.buffer().size() == 0);
}

void test_reader_array_hello()
{
  reader_t r0;
  r0.set_buffer("{\"foo\": [\"bar\", \"baz\"]}");

  test_helper_read_until_key(&r0, "foo");

  assert(is_ok(r0.read()));
  assert(event_array_begin == r0.last());
  assert(2 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_string == r0.last());
  assert(r0.raw_string() == const_range_t("bar"));

  assert(is_ok(r0.read()));
  assert(event_string == r0.last());
  assert(r0.raw_string() == const_range_t("baz"));

  assert(is_ok(r0.read()));
  assert(event_array_end == r0.last());
  assert(1 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(0 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_end == r0.last());

  assert(r0.buffer().size() == 0);
}

void test_reader_array_of_array()
{
  reader_t r0;
  r0.set_buffer("{\"foo\": [[\"baz\"]]}");

  test_helper_read_until_key(&r0, "foo");

  assert(is_ok(r0.read()));
  assert(event_array_begin == r0.last());
  assert(2 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_array_begin == r0.last());
  assert(3 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_string == r0.last());
  assert(r0.raw_string() == const_range_t("baz"));

  assert(is_ok(r0.read()));
  assert(event_array_end == r0.last());
  assert(2 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_array_end == r0.last());
  assert(1 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(0 == r0.scope_depth());

  assert(is_ok(r0.read()));
  assert(event_end == r0.last());

  assert(r0.buffer().size() == 0);
}

void test_helper_reader_object_value_predicate(bool pred, const char* str)
{
  reader_t r0;
  r0.set_buffer(str);

  test_helper_read_until_key(&r0, "foo");

  assert(is_ok(r0.read()));
  assert(event_predicate == r0.last());
  assert(r0.predicate() == pred);
  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(r0.buffer().size() == 0);
}

void test_reader_object_value_predicate()
{
  test_helper_reader_object_value_predicate(true, "{\"foo\": true}");
  test_helper_reader_object_value_predicate(false, "{\"foo\": false}");
}

void test_reader_object_members()
{
  reader_t r0;
  r0.set_buffer("{\"foo\": \"bar\", \"hoge\": \"ika\"}");
  assert(is_ok(r0.read()));
  assert(event_object_begin == r0.last());

  assert(is_ok(r0.read()));
  assert(event_object_key == r0.last());
  assert(r0.raw_string() == const_range_t("foo"));

  assert(is_ok(r0.read()));
  assert(event_string == r0.last());
  assert(r0.raw_string() == const_range_t("bar"));

  assert(is_ok(r0.read()));
  assert(event_object_key == r0.last());
  assert(r0.raw_string() == const_range_t("hoge"));

  assert(is_ok(r0.read()));
  assert(event_string == r0.last());
  assert(r0.raw_string() == const_range_t("ika"));

  assert(is_ok(r0.read()));
  assert(event_object_end == r0.last());
  assert(r0.buffer().size() == 0);
}

void test_reader_read_number()
{
  const_range_t r;
  real_t num;

  assert(is_ok(reader_t::read_number(const_range_t("1"), &r, &num)));
  assert(1.0f == num);
  assert(r.size() == 0);

  assert(is_ok(reader_t::read_number(const_range_t("2}"), &r, &num)));
  assert(2.0f == num);
  assert(r.size() == 1);

  assert(is_ok(reader_t::read_number(const_range_t(" 3.00,4"), &r, &num)));
  assert(3.0f == num);
  assert(r.size() == 2);

  assert(is_ok(reader_t::read_number(const_range_t(" -1.5,4"), &r, &num)));
  assert(-1.5f == num);
  assert(r.size() == 2);

  assert(!is_ok(reader_t::read_number(const_range_t(" -,"), &r, &num)));
  assert(!is_ok(reader_t::read_number(const_range_t(" "), &r, &num)));
  // curiously, rfc4627 does not allow followings.
  assert(!is_ok(reader_t::read_number(const_range_t("+10"), &r, &num)));
  assert(!is_ok(reader_t::read_number(const_range_t(".0"), &r, &num)));
}

void test_reader_unescape()
{
  char to_unescape[] = "hello\\nworld";
  char expected[] = "hello\nworld";
  char buf[128];
  size_t written;
  assert(is_ok(unescape(const_range_t(to_unescape), buf, 128, &written)));
  assert(0 == strcmp(buf, expected));
  assert(strlen(expected)+1 == written);
}

void test_reader_escape()
{
  char to_escape[] = "hello\nworld";
  char expected[] = "hello\\nworld";
  char buf[128];
  size_t written;
  assert(is_ok(escape(const_range_t(to_escape), buf, 128, &written)));
  assert(0 == strcmp(buf, expected));
  assert(strlen(expected)+1 == written);
}

void test_reader_unescape_unicode()
{
  char buf[128];
  size_t written;

  // http://www.fileformat.info/info/unicode/char/00a9/index.htm
  char target0[] = "\\u00A9";
  char expect0[] = {char(0xC2), char(0xA9), char(0x00)};
  assert(is_ok(unescape(const_range_t(target0), buf, 128, &written)));
  assert(2+1 == written); // + 1 for null-char
  assert(0 == strcmp(buf, expect0));

  // http://www.fileformat.info/info/unicode/char/3042/index.htm
  char target1[] = "\\u3042";
  char expect1[] = {char(0xE3), char(0x81), char(0x82), char(0x00)};
  assert(is_ok(unescape(const_range_t(target1), buf, 128, &written)));
  assert(3+1 == written);
  assert(0 == strcmp(buf, expect1));

  // http://www.fileformat.info/info/unicode/char/0041/index.htm
  char target2[] = "\\u0041";
  char expect2[] = {char(0x41), char(0x00)};
  assert(is_ok(unescape(const_range_t(target2), buf, 128, &written)));
  assert(1+1 == written);
  assert(0 == strcmp(buf, expect2));
}

void test_reader_escape_unicode()
{
  /*
   * escape() should not change multibyte characters
   */
  char buf[128];
  size_t written;

  // http://www.fileformat.info/info/unicode/char/00a9/index.htm
  char target0[] = {char(0xC2), char(0xA9), char(0x00)};
  assert(is_ok(escape(const_range_t(target0), buf, 128, &written)));
  assert(2+1 == written); // + 1 for null-char
  assert(0 == strcmp(buf, target0));

  // http://www.fileformat.info/info/unicode/char/3042/index.htm
  char target1[] = {char(0xE3), char(0x81), char(0x82), char(0x00)};
  assert(is_ok(escape(const_range_t(target1), buf, 128, &written)));
  assert(3+1 == written);
  assert(0 == strcmp(buf, target1));

  // http://www.fileformat.info/info/unicode/char/0041/index.htm
  char target2[] = {char(0x41), char(0x00)};
  assert(is_ok(escape(const_range_t(target2), buf, 128, &written)));
  assert(1+1 == written);
  assert(0 == strcmp(buf, target2));
}

void test_count_escape_overhead()
{
  assert(0 == count_escape_overhead(""));
  assert(0 == count_escape_overhead("a"));
  assert(1 == count_escape_overhead("\t"));
  assert(1 == count_escape_overhead("a\tb"));
}

void test_read_ok(const char* str)
{
  reader_t reader;
  reader.set_buffer(str, strlen(str));
  while (event_end != reader.last()) {

	error_e e = reader.read();
	assert(is_ok(e));
  }
}

void test_read_ok_list()
{
  test_read_ok("{\"foo\":\n\"bar\"}");
}

void test_reader()
{
  test_const_range_eq();
  test_reader_hello();
  test_reader_object_value_string();
  test_reader_object_value_number();
  test_reader_object_members();
  test_reader_read_number();
  test_reader_object_value_predicate();
  test_reader_object_value_null();
  test_reader_object_value_object();
  test_reader_array_hello();
  test_reader_array_of_array();
  test_reader_object_value_string_mb();
  test_reader_unescape();
  test_reader_unescape_unicode();
  test_reader_escape();
  test_reader_escape_unicode();
  test_count_escape_overhead();
  test_read_ok_list();
}

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
