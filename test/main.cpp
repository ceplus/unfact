
/*
 * unit test driver.
 *
 * each tests are placed on xxx_test.cpp, that define entrypoint named test_xxx().
 */

void test_unit(); // in unit_test.cpp

/* unfact */
void test_memory();// in unfact_memory_test.cpp
void test_arena(); // in unfact_arena_test.cpp
void test_tree_set(); // in unfact_tree_set_test.cpp
void test_red_black(); // in unfact_red_black_test.cpp
void test_base(); // in unfact_base_test.cpp
void test_algorithm(); // in unfact_algorithm_test.cpp
void test_set_tree(); // in unfact_set_tree_test.cpp
void test_static_string(); // in unfact_static_string_test.cpp
void test_tracing(); // in unfact_tracing_test.cpp
void test_heap_tracer(); // in unfact_heap_tracer_test.cpp
void test_concurrent(); // in unfact_concurrent_test.cpp
void test_tls(); // in unfact_tls_test.cpp
void test_heap_tracing_annotation(); // in unfact_heap_tracing_annotation_test.cpp
void test_sticky_tracer(); // in unfact_sticky_tracer_test.cpp

/* ontree */
void test_reader(); // in reader_test.cpp
void test_tree(); // in tree_test.cpp
void test_builder();  // in builder_test.cpp
void test_writer(); // in writer_test.cpp
void test_generator(); // in generator_test.cpp
void test_var(); // in var_test.cpp
void test_find(); // in find_test.cpp
void hello_ontree(); // in hello_ontree.cpp

int main(int /*argc*/, char* /*argv*/[])
{
  test_unit();

  /* unfact */
  test_base();
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

  /* ontree */
  test_reader();
  test_tree();
  test_builder();
  test_writer();
  test_generator();
  test_var();
  test_find();
  hello_ontree();
  
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
