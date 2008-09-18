
#include <test/unit.hpp>
#include <cstdlib>
#include <cstdio>

bool unfact_test_failed(const char* file, size_t line, const char* cond)
{
	fprintf(stderr, "%s:%d: %s\n", file, line, cond);
	abort();
	return true;
}

