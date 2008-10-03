
#ifndef ONTREE_WRITER_HPP
#define ONTREE_WRITER_HPP

#include <ontree/base.hpp>
#include <ontree/event.hpp>
#include <ontree/strings.hpp>
#include <ontree/bufstream.hpp>
#include <cstring>
#include <cstdlib>
#include <cctype>

ONT_NAMESPACE_BEGIN

#define ONT_WRITER_RETURN_UNLESS_OK(err, exp) if (error_ok != (err = (exp))) return err

struct counted_scope_t
{
  scope_e m_type;
  size_t  m_count;
};

class writer_t
{
public:
  enum { scope_depth_limit = 32 };

  writer_t() : m_scope_depth(0), m_last(event_begin) {}

  void set_buffer(char* head, size_t len) { m_buffer = range_t(head, len); }
  const range_t& buffer() const { return m_buffer; }

  error_e write_begin(scope_e scope)
  {
	if (!value_available() && 
		(m_scope_depth == 0 && scope != scope_object)) {
	  return error_ill_formed;
	}

	error_e err = error_ok;
	switch (scope)
	{
	case scope_object:
	  ONT_WRITER_RETURN_UNLESS_OK(err, write_char(m_buffer, &m_buffer, '{'));
	  break;
	case scope_array:
	  ONT_WRITER_RETURN_UNLESS_OK(err, write_char(m_buffer, &m_buffer, '['));
	  break;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  break;
	}

	ONT_WRITER_RETURN_UNLESS_OK(err, push_scope(scope));
	m_last = (scope_object == scope) ? event_object_begin : event_array_begin;

	return error_ok;
  }

  error_e write_end()
  {
	if (0 == m_scope_depth ||
		event_object_key == m_last) {
	  return error_ill_formed;
	}

	error_e err = error_ok;
	scope_e scope = top_scope();
	event_e here = event_end;

	switch (scope)
	{
	case scope_object:
	  ONT_WRITER_RETURN_UNLESS_OK(err, write_char(m_buffer, &m_buffer, '}'));
	  here = event_object_end;
	  break;
	case scope_array:
	  ONT_WRITER_RETURN_UNLESS_OK(err, write_char(m_buffer, &m_buffer, ']'));
	  here = event_array_end;
	  break;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  break;
	}

	ONT_WRITER_RETURN_UNLESS_OK(err, pop_scope());
	m_last = here;

	return error_ok;
  }

  error_e write_key(const char* name)
  {
	if (0 == m_scope_depth ||
		top_scope() == scope_array ||
		event_object_key == m_last) {
	  return error_ill_formed;
	}

	error_e err = error_ok;
	range_t r;

	ONT_WRITER_RETURN_UNLESS_OK(err, write_value_separator(m_buffer, &r, event_object_key));
	ONT_WRITER_RETURN_UNLESS_OK(err, write_string(r, &r, name));
	ONT_WRITER_RETURN_UNLESS_OK(err, write_char(r, &m_buffer, ':'));
	m_last = event_object_key;

	return error_ok;
  }


  template<class T>
  error_e write_value(const T& value)
  {
	if (!value_available()) {
	  return error_ill_formed;
	}

	error_e err = error_ok;
	range_t r;
	ONT_WRITER_RETURN_UNLESS_OK(err, write_value_separator(m_buffer, &r, event_string));
	ONT_WRITER_RETURN_UNLESS_OK(err, write_value_literal(r, &m_buffer, value));
	countup();
	m_last = event_value;

	return error_ok;
  }

public: // export just for testing
  error_e write_value_separator(const range_t& from, range_t* to, event_e ev)
  {
	if (!value_separator_requried(ev)) {
	  *to = from;
	  return error_ok;
	}

	error_e err = error_ok;
	ONT_WRITER_RETURN_UNLESS_OK(err, write_char(from, to, ','));
	return error_ok;
  }

  bool value_available() const
  {
	return (0 < m_scope_depth &&
			(top_scope() == scope_array ||
			 event_object_key == m_last));
  }

  bool value_separator_requried(event_e ev) const
  {
	if (0 == top_count()) {
	  return false;
	}

	switch (top_scope()) {
	case scope_object:
	  return (ev == event_object_key);
	case scope_array:
	  return true;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  return false;
	}
  }

  error_e push_scope(scope_e s)
  {
	if(scope_depth_limit <= m_scope_depth) {
	  return error_too_deep;
	}

	m_scopes[m_scope_depth].m_type  = s;
	m_scopes[m_scope_depth].m_count = 0;
	m_scope_depth++;

	return error_ok;
  }

  error_e pop_scope()
  {
	if(m_scope_depth == 0) {
	  return error_ill_formed;
	}

	--m_scope_depth;
	return error_ok;
  }

  scope_e top_scope() const
  {
	assert(0 < m_scope_depth);
	return m_scopes[m_scope_depth-1].m_type;
  }

  size_t top_count() const
  {
	assert(0 < m_scope_depth);
	return m_scopes[m_scope_depth-1].m_count;
  }

  void countup()
  {
	assert(0 < m_scope_depth);
	m_scopes[m_scope_depth-1].m_count++;
  }

  static error_e write_value_literal(const range_t& from, range_t* to, real_t value)
  {
	error_e err = error_ok;
	ONT_WRITER_RETURN_UNLESS_OK(err, write_number(from, to, value));
	return error_ok;
  }

  static error_e write_value_literal(const range_t& from, range_t* to, const char* str)
  {
	error_e err = error_ok;
	ONT_WRITER_RETURN_UNLESS_OK(err, write_string(from, to, str));
	return error_ok;
  }

  static error_e write_value_literal(const range_t& from, range_t* to, bool pred)
  {
	error_e err = error_ok;
	if (pred) {
	  ONT_WRITER_RETURN_UNLESS_OK(err, write_word(from, to, "true"));
	} else {
	  ONT_WRITER_RETURN_UNLESS_OK(err, write_word(from, to, "false"));
	}
	return error_ok;
  }

  static error_e write_value_literal(const range_t& from, range_t* to, const none_t& null)
  {
	error_e err = error_ok;
	ONT_WRITER_RETURN_UNLESS_OK(err, write_word(from, to, "null"));
	return error_ok;
  }

  static error_e write_char(const range_t& from, range_t* to, char ch)
  {
	if (0 == from.size()) {
	  return error_need_buffer;
	}

	*to = from.write(ch);
	return error_ok;
  }

  static error_e write_string(const range_t& from, range_t* to, const char* str)
  {
	size_t len  = strlen(str);
	size_t oh   = count_escape_overhead(str, len);
	size_t need = len + oh + 2; // +2 for and quotes

	if (from.size() < need) {
	  return error_need_buffer;
	}

	error_e err = error_ok;
	size_t written = 0;
	range_t r;

	r   = from.write('"');
	err = escape(const_range_t(str, len), r.head(), r.size(), &written);
	if (!is_ok(err)) {
	  return err;
	}
		
	r.consume(written-1); // -1 for canceling '\0'
	*to = r.write('"');

	return error_ok;
  }

  static error_e write_number(const range_t& from, range_t* to, real_t value)
  {
	char buf[64];
	obufstream_t out(buf, 64);

	out << value;
	assert(!out.overflowed());

	if (from.size() < out.written()) {
	  return error_need_buffer;
	}

	*to = from.write(out.buffer(), out.written());
	return error_ok;
  }

  static error_e write_word(const range_t& from, range_t* to, const char* word)
  {
	size_t wlen  = strlen(word);
	if (from.size() < wlen) {
	  return error_need_buffer;
	}

	*to = from.write(word, wlen);
	return error_ok;
  }

private:
  range_t m_buffer;
  counted_scope_t m_scopes[scope_depth_limit];
  size_t m_scope_depth;
  event_e m_last;
};

ONT_NAMESPACE_END

#endif//ONTREE_WRITER_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
