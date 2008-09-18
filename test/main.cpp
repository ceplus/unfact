
/*
 * unit test driver.
 *
 * each tests are placed on xxx_test.cpp, that define entrypoint named test_xxx().
 */

void test_unit(); // in unit_test.cpp
void test_memory();// in memory_test.cpp
void test_arena(); // in arena_test.cpp
void test_tree_set(); // in tree_set_test.cpp
void test_red_black(); // in red_black_test.cpp
void test_base(); // in base_test.cpp
void test_algorithm(); // in algorithm_test.cpp
void test_set_tree(); // in set_tree_test.cpp
void test_static_string(); // in static_string_test.cpp
void test_tracing(); // in tracing_test.cpp
void test_heap_tracer(); // in heap_tracer_test.cpp
void test_concurrent(); // in concurrent_test.cpp
void test_tls(); // in tls_test.cpp
void test_heap_tracing_annotation(); // in heap_tracing_annotation_test.cpp
void test_sticky_tracer(); // in sticky_tracer_test.cpp

int main(int /*argc*/, char* /*argv*/[])
{
  test_base();
  test_unit();
  test_memory();
  test_algorithm();
  test_arena();
  test_red_black();
  test_tree_set();
  test_set_tree();
  test_static_string();
  test_tracing();
  test_heap_tracer();
  test_concurrent();
  test_tls();
  test_heap_tracing_annotation();
  test_sticky_tracer();
  return 0;
}

/* -*-
 Local Variables:
 mode: c++
 c-tab-always-indent: t
 c-indent-level: 2
 c-basic-offset: 2
 End:
 -*- */
