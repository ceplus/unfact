
#ifndef ONTREE_READER_HPP
#define ONTREE_READER_HPP

#include <ontree/base.hpp>
#include <ontree/event.hpp>
#include <ontree/strings.hpp>
#include <cstring>
#include <cstdlib>
#include <cctype>

ONT_NAMESPACE_BEGIN

#define ONT_READER_RETURN_UNLESS_OK(err, exp) if (error_ok != (err = (exp))) return err

class reader_t
{
public:
  enum { scope_depth_limit = 32 };

  reader_t() 
	: m_scope_depth(0), m_last(event_begin),
	  m_number(0.0f), m_predicate(false)
  {}

  void set_buffer(const char* head, size_t len) { m_buffer = const_range_t(head, len); }
  void set_buffer(const char* head) { m_buffer = const_range_t(head); }
  const const_range_t& buffer() const { return m_buffer; }

  event_e last() const { return m_last; }

  const const_range_t& raw_string() const { return m_raw_string; }
  real_t number() const { return m_number; }
  bool predicate() const { return m_predicate; }

  error_e read()
  {
	error_e err = error_unexpected;
	switch (m_last)
	{
	case event_begin:
	  err = read_object_begin(m_buffer, &m_buffer);
	  break;
	case event_object_begin:
	  err = read_first_object_key_or_end(m_buffer, &m_buffer);
	  break;
	case event_object_key:
	  err = read_object_value(m_buffer, &m_buffer);
	  break;
	case event_array_begin:
	  err = read_first_array_value_or_end(m_buffer, &m_buffer);
	  break;
	case event_array_end:
	case event_object_end:
	case event_string:
	case event_number:
	case event_predicate:
	case event_null:
	  if (0 == m_scope_depth) {
		err = error_ok;
		m_last = event_end;
	  } else {
		switch (current_scope())
		{
		case scope_object:
		  err = read_following_object_key_or_end(m_buffer, &m_buffer);
		  break;
		case scope_array:
		  err = read_following_array_value_or_end(m_buffer, &m_buffer);
		  break;
		default:
		  ONT_SHOULD_NOT_BE_REACHED();
		  break;
		}
	  }
	  break;
	case event_end:
	  ONT_SHOULD_NOT_BE_REACHED();
	  break;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  break;
	}

	return err;
  }

public: // export for testing
  static error_e read_whitespaces(const const_range_t& from, const_range_t* to)
  {
	size_t i = 0;
	for (/* */; i < from.size(); ++i) {
	  char ch = from.at(i);
	  if (ch == ' '  ||
		  ch == '\t' ||
		  ch == '\v' ||
		  ch == '\n' ||
		  ch == '\r') {
		/* read more */
	  } else {
		break;
	  }
	}

	*to = from.consumed(i);
	return error_ok;
  }

  static error_e read_char(const const_range_t& from, const_range_t* to, char ch)
  {
	if (0 == from.size()) {
	  return error_need_buffer;
	}

	if (ch != from.at(0)) {
	  return error_ill_formed;
	}
		
	*to = from.consumed(1);

	return error_ok;
  }

  static error_e read_word(const const_range_t& from, const_range_t* to, const char* str)
  {
	size_t i=0;
	while (str[i] && i < from.size()) {
	  if (str[i] != from.at(i)) {
		return error_ill_formed;
	  }
	  ++i;
	}

	*to = from.consumed(i);
	return error_ok;
  }


  static error_e read_hexdigits(const const_range_t& from, const_range_t* to, size_t len)
  {
	if (from.size() < len) {
	  return error_need_buffer;
	}

	size_t i=0;
	for (/* */; i<len; ++i) {
	  char ch = from.at(i);
	  if (isdigit(ch) || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F')) {
		/* contiue; */
	  } else {
		return error_ill_formed;
	  }
	}

	*to = from.consumed(i);
	return error_ok;
  }

  static error_e read_value_separator(const const_range_t& from, const_range_t* to)
  {
	const_range_t r;
	error_e err;

	// value-separator = ws %x2C ws  ; , comma
	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(from, &r));
	ONT_READER_RETURN_UNLESS_OK(err, read_char(r, &r, ','));
	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(r, to));

	return error_ok;
  }

  static error_e read_string(const const_range_t& from, const_range_t* to, const_range_t* str)
  {
	error_e err;
	const_range_t r;

	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(from, &r));
	ONT_READER_RETURN_UNLESS_OK(err, read_char(r, &r, '"'));

	bool terminatd = false;
	size_t n = 0;
	while (n < r.size()) {

//          char = unescaped /
//                 escape (
//                     %x22 /          ; "    quotation mark  U+0022
//                     %x5C /          ; \    reverse solidus U+005C
//                     %x2F /          ; /    solidus         U+002F
//                     %x62 /          ; b    backspace       U+0008
//                     %x66 /          ; f    form feed       U+000C
//                     %x6E /          ; n    line feed       U+000A
//                     %x72 /          ; r    carriage return U+000D
//                     %x74 /          ; t    tab             U+0009
//                     %x75 4HEXDIG )  ; uXXXX                U+XXXX
//          escape = %x5C              ; \

	  char ch = r.at(n);
	  if ('"' == ch) {
		terminatd = true;
		break;
	  } else if ('\\' == ch) {

		n++;
		if (r.size() <= n) {
		  break;
		}

		ch = r.at(n);
		switch (ch) {
		case '"':
		case '\\':
		case '/':
		case 'b':
		case 'f':
		case 'n':
		case 'r':
		case 't':
		  n++;
		  break;
		case 'u': {
		  n++;
		  const_range_t d;
		  ONT_READER_RETURN_UNLESS_OK(err, read_hexdigits(const_range_t(r.head() + n, r.size() - n), &d, 4));
		  n += 4;
		}	break;
		default:
		  return error_ill_formed;
		}
	  } else {
		size_t chlen = utf8_char_size(r.at(n));
		if (0 == chlen) {
		  return error_ill_formed;
		}
		if (r.size() < n + chlen) {
		  return error_need_buffer;
		}

		n += chlen;
	  }

	}

	if (!terminatd) {
	  return error_need_buffer;
	}

	const_range_t trail = r.consumed(n);

	ONT_READER_RETURN_UNLESS_OK(err, read_char(trail, &trail, '"'));
	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(trail, to));

	*str = const_range_t(r.head(), n);

	return error_ok;
  }

  static error_e read_number(const const_range_t& from, const_range_t* to, real_t* num)
  {
	error_e err;
	const_range_t r;

	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(from, &r));

	if (0 == r.size()) {
	  return error_need_buffer;
	}

	// number = [ minus ] int [ frac ] [ exp ]
	char ch = r.at(0);
	if (!isdigit(ch) && '-' != ch) {
	  return error_ill_formed;
	}

	/* 
	 * make temporal null-terminated string that strtod() requires.
	 */
	char buf[32];
	size_t len = min_of((32U-1U), r.size());
	memcpy(buf, r.head(), len);
	buf[len] = '\0';
	char* out = 0;
	real_t val = static_cast<real_t>(strtod(buf, &out));
	size_t passed = out - buf;
	if (0 == passed) {
	  return error_ill_formed;
	}

	*num = val;
	*to = r.consumed(passed);
	return error_ok;
  }

  error_e read_predicate(const const_range_t& from, const_range_t* to, bool* pred)
  {
	error_e err;
	err = read_word(from, to, "true");
	if (is_ok(err)) {
	  *pred = true;
	  return error_ok;
	}

	err = read_word(from, to, "false");
	if (is_ok(err)) {
	  *pred = false;
	  return error_ok;
	}

	return err;
  }

  error_e read_null(const const_range_t& from, const_range_t* to, bool* pred)
  {
	return read_word(from, to, "null");
  }

  error_e read_begin(const const_range_t& from, const_range_t* to,
					 event_e next_event, char brace, scope_e scope)
  {
	if (scope_depth_limit <= m_scope_depth) {
	  return error_too_deep;
	}

	error_e err;
	const_range_t r;
	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(from, &r));
	ONT_READER_RETURN_UNLESS_OK(err, read_char(r, &r, brace));
	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(r, to));

	m_last = next_event;
	push_scope(scope);

	return error_ok;
  }


  error_e read_end(const const_range_t& from, const_range_t* to,
				   event_e next_event, char brace)
  {
	assert(0 < m_scope_depth);

	error_e err;
	const_range_t r;
	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(from, &r));
	ONT_READER_RETURN_UNLESS_OK(err, read_char(r, &r, brace));
	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(r, to));

	m_last = next_event;
	pop_scope();

	return error_ok;
  }

  error_e read_object_begin(const const_range_t& from, const_range_t* to)
  {
	return read_begin(from, to, event_object_begin, '{', scope_object);
  }


  error_e read_object_end(const const_range_t& from, const_range_t* to)
  {
	return read_end(from, to, event_object_end, '}');
  }

  error_e read_object_key(const const_range_t& from, const_range_t* to)
  {
	error_e err;
	const_range_t str;
	const_range_t r;


	// member = string name-separator value
	ONT_READER_RETURN_UNLESS_OK(err, read_string(from, &r, &str));
	// name-separator  = ws %x3A ws  ; : colon
	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(r, &r));
	ONT_READER_RETURN_UNLESS_OK(err, read_char(r, &r, ':'));
	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(r, to));

	m_raw_string = str;
	m_last = event_object_key;

	return error_ok;
  }

  error_e read_first_object_key_or_end(const const_range_t& from, const_range_t* to)
  {
	const_range_t r = from;
	error_e err;
		
	err = read_object_key(from, &r);
	if (error_ill_formed != err) {
	  if (is_ok(err)) { *to = r; }
	  return err;
	}

	return read_object_end(r, to);
  }

  error_e read_following_object_key_or_end(const const_range_t& from, const_range_t* to)
  {
	error_e err;
	const_range_t r;
		
	err = read_value_separator(from, &r);
	if (is_ok(err)) {
	  return read_object_key(r, to);
	} else {
	  return read_object_end(from, to);
	}
  }

  error_e read_array_begin(const const_range_t& from, const_range_t* to)
  {
	return read_begin(from, to, event_array_begin, '[', scope_array);
  }


  error_e read_array_end(const const_range_t& from, const_range_t* to)
  {
	return read_end(from, to, event_array_end, ']');
  }

  error_e read_first_array_value_or_end(const const_range_t& from, const_range_t* to)
  {
	const_range_t r = from;
	error_e err;
		
	err = read_object_value(from, &r);
	if (error_ill_formed != err) {
	  if (is_ok(err)) { *to = r; }
	  return err;
	}

	return read_array_end(r, to);
  }

  error_e read_following_array_value_or_end(const const_range_t& from, const_range_t* to)
  {
	error_e err;
	const_range_t r;
		
	err = read_value_separator(from, &r);
	if (is_ok(err)) {
	  return read_object_value(r, to);
	} else {
	  return read_array_end(from, to);
	}
  }

  error_e read_object_value(const const_range_t& from, const_range_t* to)
  {
	error_e err;
	const_range_t r;

	ONT_READER_RETURN_UNLESS_OK(err, read_whitespaces(from, &r));
		
	err = read_string(r, to, &m_raw_string);
	if (error_ok == err || error_ill_formed != err) {
	  if (is_ok(err)) {
		m_last = event_string;
	  }
	  return err;
	}

	err = read_number(r, to, &m_number);
	if (error_ok == err || error_ill_formed != err) {
	  if (is_ok(err)) {
		m_last = event_number;
	  }
	  return err;
	}

	err = read_predicate(r, to, &m_predicate);
	if (error_ok == err || error_ill_formed != err) {
	  if (is_ok(err)) {
		m_last = event_predicate;
	  }
	  return err;
	}

	err = read_null(r, to, &m_predicate);
	if (error_ok == err || error_ill_formed != err) {
	  if (is_ok(err)) {
		m_last = event_null;
	  }
	  return err;
	}

	err = read_object_begin(r, to);
	if (error_ok == err || error_ill_formed != err) {
	  // state is updated inside read_object_begin()
	  return err;
	}

	err = read_array_begin(r, to);
	if (error_ok != err) {
	  // state is updated inside read_array_begin()
	  return err;
	}

	return error_ok;
  }

public: // just for testing
  void set_last(event_e e) { m_last = e; }
  size_t scope_depth() const { return m_scope_depth; }

  void push_scope(scope_e scope)
  {
	assert(m_scope_depth < scope_depth_limit);
	m_scopes[m_scope_depth++] = scope;
  }

  void pop_scope()
  {
	assert(0 < m_scope_depth);
	m_scope_depth--;
  }

  scope_e current_scope() const
  { 
	assert(0 < m_scope_depth);
	return m_scopes[m_scope_depth-1];
  }
private:
  const_range_t m_buffer;

  event_e m_last;
  scope_e m_scopes[scope_depth_limit];
  size_t m_scope_depth;

  /*
   * values
   */
  const_range_t m_raw_string;
  real_t m_number;
  bool m_predicate;
};

ONT_NAMESPACE_END

#endif//ONTREE_READER_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
