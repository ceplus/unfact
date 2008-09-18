
/*
 * this file is included onluy from log.hpp, and should NOT be included directly.
 */

#ifdef  UNFACT_HAS_USER_EXTENSION
# define UNFACT_DEFINING_USER_EXTENSION_LOG
# include <unfact_user_extension.hpp>
# undef  UNFACT_DEFINING_USER_EXTENSION_LOG
#endif//UNFACT_HAS_USER_EXTENSION

/*
 * each user can define their own unfact_log_printf(): 
 * - define unfact_log_printf() somewhere 
 * - declare it in "unfact_user_extension.hpp"
 * - define "UNFACT_HAS_USER_LOGGER" accordingly
 */

#ifndef UNFACT_HAS_USER_LOG_PRINT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unfact/platform/filename.hpp>

UNFACT_NAMESPACE_BEGIN

inline void 
log_printf(const char* file, size_t line, unfact::log_level_e lv, 
		   const char* format, va_list ap)
{
  namespace uf = unfact;

  fprintf(stderr, "%s:%d:UNFACT[%s] ", strip_directory(file), line, uf::log_level_str(lv));
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");

  if (uf::log_level_fatal == lv) {
	fprintf(stderr, "abort!\n");
	abort();
  }

}

UNFACT_NAMESPACE_END

#endif//UNFACT_HAS_USER_LOG_PRINT

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
