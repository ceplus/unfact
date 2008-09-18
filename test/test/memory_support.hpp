
#ifndef UNFACT_TEST_MEMORY_SUPPORT_HPP
#define UNFACT_TEST_MEMORY_SUPPORT_HPP

#include <unfact/memory.hpp>
#include <unfact/log.hpp>
#include <cstdlib>
#include <map>

class stdlib_allocator_t : public unfact::allocator_t
{
public:
  typedef unfact::byte_t byte_t;

  stdlib_allocator_t() {}

  virtual byte_t* allocate(size_t size) { return reinterpret_cast<byte_t*>(malloc(size)); }
  virtual void deallocate(byte_t* ptr) { free(ptr); }

private:
  stdlib_allocator_t(const stdlib_allocator_t&);
  const stdlib_allocator_t& operator=(stdlib_allocator_t&);
};

class tracing_allocator_t : public unfact::allocator_t
{
public:
  typedef unfact::byte_t byte_t;
  typedef std::map<byte_t*, size_t> map_type;

  tracing_allocator_t(bool fol=true) : m_fail_on_leak(fol) {}

  virtual ~tracing_allocator_t() {
	if (!m_allocations.empty()) {
	  if (m_fail_on_leak) {
		UF_ASSERT(!("found possible memory leak!"));
	  } else {
		UF_ALERT(("found possible memory leak!"));
	  }
	}
  }

  virtual byte_t* allocate(size_t size) {
	byte_t* ret = reinterpret_cast<byte_t*>(malloc(size));
	if (0 == ret) {
	  return 0;
	}

	m_allocations[ret] = size;
	return ret;
  }

  virtual void deallocate(byte_t* ptr) {
	m_allocations.erase(ptr);
	free(ptr);
  }

private:
  tracing_allocator_t(const tracing_allocator_t&);
  const tracing_allocator_t& operator=(tracing_allocator_t&);
	
  map_type m_allocations;
  bool m_fail_on_leak;
};

class fail_allocator_t : public unfact::allocator_t
{
public:
  typedef unfact::byte_t byte_t;

  explicit fail_allocator_t(size_t fail_aftr) : m_fail_after(fail_aftr) {}

  virtual byte_t* allocate(size_t size)
  {
	if (0 == m_fail_after) {
	  return 0;
	}

	m_fail_after--;
	return reinterpret_cast<byte_t*>(malloc(size)); 
  }

  virtual void deallocate(byte_t* ptr)
  {
	free(ptr);
  }

private:
  size_t m_fail_after;
  fail_allocator_t(const fail_allocator_t&);
  const fail_allocator_t& operator=(fail_allocator_t&);
};

#endif//UNFACT_TEST_MEMORY_SUPPORT_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
