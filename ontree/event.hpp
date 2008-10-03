
#ifndef ONTREE_EVENT_HPP
#define ONTREE_EVENT_HPP

#include <ontree/base.hpp>

ONT_NAMESPACE_BEGIN

enum event_e
{
  event_object_begin,
  event_object_end,
  event_object_key,
  event_array_begin,
  event_array_end,
  event_number,
  event_string,
  event_predicate,
  event_null,
  event_begin,
  event_end,
  event_value, // used inside writer, and never appers elsewhere
  events
};

enum scope_e
{
  scope_object,
  scope_array,
  scopes
};

ONT_NAMESPACE_END

#endif//ONTREE_EVENT_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
