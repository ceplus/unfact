
#include <unfact/static_string.hpp>
#include <test/memory_support.hpp>
#include <test/unit.hpp>
#include <vector>
#include <algorithm>
#include <string>

typedef unfact::basic_static_string_t<char> my_static_string_t;

static
void test_static_string_hello()
{
  my_static_string_t str("hello");

  UF_TEST_EQUAL(std::string("hello"), str.c_str());
  str[0] = 'H';
  UF_TEST_EQUAL(str.c_str(), std::string("Hello"));

  my_static_string_t empty;
  UF_TEST_EQUAL(empty.c_str(), std::string(""));
}

static
void test_static_string_size()
{
  my_static_string_t a("a");
  my_static_string_t empty("");
  my_static_string_t full("aaaaaaaaaaa");
  my_static_string_t overflow("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

  UF_TEST_EQUAL(a.size(), 1);
  UF_TEST_EQUAL(empty.size(), 0);
  UF_TEST_EQUAL(full.size(), 11);
  UF_TEST_EQUAL(overflow.size(), my_static_string_t::capacity);
}

static
void test_static_string_equal()
{
  my_static_string_t str0("hello");
  my_static_string_t str0b("hello");
  my_static_string_t str1("hello!");
  my_static_string_t str2("hell");

  my_static_string_t a("a");
  my_static_string_t b("b");

  UF_TEST(str0 == str0b);
  UF_TEST(!(str0 == str1));
  UF_TEST(str0 != str1);
  UF_TEST(str0 != str2);
  UF_TEST(  a < b);
  UF_TEST(!(a < a));
  UF_TEST(!(b < a));
}

void test_static_string()
{
  test_static_string_hello();
  test_static_string_equal();
}


/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
