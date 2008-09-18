
#include <unfact/memory.hpp>
#include <test/memory_support.hpp>

static void
test_stdlib_hello()
{
	stdlib_allocator_t alloc;
}

static void
test_tracing_hello()
{
	tracing_allocator_t alloc;
	unfact::byte_t* ptr0 = alloc.allocate(10);
	unfact::byte_t* ptr1 = alloc.allocate(20);
	alloc.deallocate(ptr1);
	alloc.deallocate(ptr0);
}

void test_memory()
{
	test_stdlib_hello();
	test_tracing_hello();
}
