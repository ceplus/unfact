/*
 * Copyright (c) 2008 Community Engine Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef UNFACT_LOG_HPP
#define UNFACT_LOG_HPP

#include <unfact/types.hpp>
#include <stdarg.h>


#define UF_TRACE_X(file, line, msg) unfact::log_print_t(unfact::log_level_trace, file, line) msg
#define UF_ALERT_X(file, line, msg) unfact::log_print_t(unfact::log_level_alert, file, line) msg
#define UF_ERROR_X(file, line, msg) unfact::log_print_t(unfact::log_level_error, file, line) msg
#define UF_FATAL_X(file, line, msg) unfact::log_print_t(unfact::log_level_fatal, file, line) msg
#define UF_TRACE(msg) UF_TRACE_X(__FILE__, __LINE__, msg)
#define UF_ALERT(msg) UF_ALERT_X(__FILE__, __LINE__, msg)
#define UF_ERROR(msg) UF_ERROR_X(__FILE__, __LINE__, msg)
#define UF_FATAL(msg) UF_FATAL_X(__FILE__, __LINE__, msg)

#ifdef UF_NDEBUG
# define UF_ASSERT_X(cond) (true)
# define UF_ASSERT(cond) (true)
#else
# define UF_ASSERT_X(file, line, cond) ((cond) || (UF_FATAL_X(file, line, ("assertion failed!:" #cond)), true))
# define UF_ASSERT(cond) UF_ASSERT_X(__FILE__, __LINE__, cond)
#endif

/*
 * DbC support: check if the caller honors the contract, reject to continue elsewise.
 */
#define UF_ALERT_AND_RETURN_UNLESS(cond, ret, msg) if (!(cond)) { UF_ALERT((msg)); return (ret); } 
#define UF_ALERT_AND_RETURN_VOID_UNLESS(cond, msg) if (!(cond)) { UF_ALERT((msg)); return; } 
#define UF_HONOR_OR_ALERT(contract) if (!(contract)) { UF_ERROR("contract " #contract " is broken!"); }
#define UF_HONOR_OR_RETURN(contract, ret) if (!(contract)) { UF_ERROR(("contract " #contract " is broken!")); return (ret); }
#define UF_HONOR_OR_RETURN_VOID(contract) if (!(contract)) { UF_ERROR(("contract " #contract " is broken!")); return; }
#define UF_NOT_IMPLEMENTED_YET() UF_ASSERT(!"Not Implemented Yet!")

UNFACT_NAMESPACE_BEGIN

/*
 * log levels. our policy follows:
 * 
 * - TRACE: Should be used just for debugging purpose on development, and
 *          removed before release
 *
 * - ALERT: Notify when some trouble happens.
 *          ALERT should be possible even if your program is correct.
 *          A typical ALERT is for memory shortage.
 *
 * - ERROR: Notify something wrong happens. 
 *          This should NOT be raised when your program behaves correctly.
 *          In other word, any ERROR notification should be a bug, and fixed immediately.
 *
 * - FATAL: Notify something wrong, the will crash programs happen.
 *          On FATAL condition, program should have no cange but crash.
 *          Because ignoraing it will lead some vulnerabilities.
 *          So our default logging implementation abort() when it goes FATAL.
 *
 * You might say that ERROR and FATAL seem to have similar responsibility.
 * Here is some explanation:
 *   Basically, we have no right for kill your program, although there is a necessary evil.
 *   so we use ERROR for wrong usage on public API (that you could make),
 *   and FATAL for internal invariant breakages.
 */
enum log_level_e
{
  log_level_trace = 0,
  log_level_alert,
  log_level_error,
  log_level_fatal, /* for assertion failure */
  log_levels
};

inline const char* log_level_str(log_level_e lv)
{
  switch (lv)
  {
  case log_level_trace:
		return "TRACE";
		break;
  case log_level_alert:
		return "ALERT";
		break;
  case log_level_error:
		return "ERROR";
  case log_level_fatal:
		return "FATAL";
  default:
		return "?????";
  }
}

/*
 * user-defined log printing function.
 * standard implementation using stdio is defined at unfact_log.cpp
 */
void log_printf(const char* file, size_t line, log_level_e lv,
								const char* format, va_list ap);

class log_print_t
{
public:
  log_print_t(log_level_e lv, const char* file, int line)
		: m_lv(lv), m_file(file), m_line(line) {}
  void operator()(const char* format, ...)
	{
		va_list vl;
		va_start(vl, format);
		log_printf(m_file, m_line, m_lv, format, vl);
		va_end(vl);
	}
private:
  log_level_e m_lv;
  const char* m_file;
  int         m_line;
};

UNFACT_NAMESPACE_END

#include <unfact/log.ipp>

#endif//UNFACT_LOG_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   tab-width: 2
   End:
   -*- */
