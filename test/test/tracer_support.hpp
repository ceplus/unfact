
#ifndef UNFACT_TEST_TRACER_SUPPORT_HPP
#define UNFACT_TEST_TRACER_SUPPORT_HPP

#include <unfact/memory.hpp>
#include <unfact/log.hpp>
#include <string>

template<class Tracer>
inline std::string
qualified_tracing_name(const Tracer& tracer, 
					   typename Tracer::ticket_type here)
{
  std::string ret;
  typedef typename Tracer::ticket_type ticket_type;
  for (ticket_type i=here; i != tracer.root(); i = tracer.parent(i)) {
	if (ret.empty()) {
	  ret = std::string(tracer.name_of(i).c_str());
	} else {
	  ret = std::string(tracer.name_of(i).c_str()) + "." + ret;
	}
  }

  return ret;
}

#endif//UNFACT_TEST_TRACER_SUPPORT_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
