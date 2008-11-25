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

#ifndef ONTREE_BUILDER_HPP
#define ONTREE_BUILDER_HPP

#include <ontree/reader.hpp>
#include <ontree/tree.hpp>

ONT_NAMESPACE_BEGIN

class builder_t
{
public:
  explicit builder_t(tree_t* t) : m_tree(t), m_top(0) {}

  template<class Reader>
  void build(const Reader& reader)
  {
	error_e err = error_ok;

	switch(reader.last())
	{
	case event_object_begin:
	  build_begin(type_object);
	  break;
	case event_array_begin:
	  build_begin(type_array);
	  break;
	case event_object_end:
	case event_array_end:
	  build_object_or_array_end();
	  break;
	case event_object_key: {
	  size_t n = 0;
	  m_last_name = m_tree->new_text(reader.raw_string().size());
	  err = unescape(reader.raw_string(), m_last_name.data(), m_last_name.capacity()+1, &n);
	  assert(is_ok(err));
	}   break;
	case event_number:
	  build_leaf(reader.number());
	  break;
	case event_string: {
	  size_t n = 0;
	  text_t t = m_tree->new_text(reader.raw_string().size());
	  err = unescape(reader.raw_string(), t.data(), t.capacity()+1, &n);
	  assert(is_ok(err));
	  build_leaf(t);
	}   break;
	case event_predicate:
	  build_leaf(reader.predicate());
	  break;
	case event_null:
	  build_leaf(none_t());
	  break;
	case event_begin:
	case event_end:
	  break;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  break;
	}
  }

private:
  void build_begin(type_e type)
  {
	if (m_top) {
	  switch (m_top->type()) {
	  case type_object:
		switch (type) {
		case type_object:
		  m_top = m_tree->insert_object(static_cast<object_t*>(m_top), m_last_name);
		  break;
		case type_array:
		  m_top = m_tree->insert_array(static_cast<object_t*>(m_top), m_last_name);
		  break;
		default:
		  ONT_SHOULD_NOT_BE_REACHED();
		  break;
		}
		return;
	  case type_array:
		switch (type) {
		case type_object:
		  m_top = m_tree->insert_object(static_cast<array_t*>(m_top));
		  break;
		case type_array:
		  m_top = m_tree->insert_array(static_cast<array_t*>(m_top));
		  break;
		default:
		  ONT_SHOULD_NOT_BE_REACHED();
		  break;
		}
		return;
	  default:
		ONT_SHOULD_NOT_BE_REACHED();
		return;
	  }
	} else {
	  m_top = m_tree->root();
	}
  }

  template<class T>
  void build_leaf(const T& val)
  {
	switch (m_top->type()) {
	case type_object:
	  m_tree->insert_leaf(static_cast<object_t*>(m_top), m_last_name, val);
	  break;
	case type_array:
	  m_tree->insert_leaf(static_cast<array_t*>(m_top), val);
	  break;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  return;
	}
  }

  void build_object_or_array_end()
  {
	m_top = m_top->parent();
  }

private:
  tree_t* m_tree;
  node_t* m_top;
  text_t  m_last_name;
};

inline error_e build_tree(tree_t* tree, const char* str, size_t len)
{
  builder_t builder(tree);
  reader_t reader;
  reader.set_buffer(str, len);
  while (event_end != reader.last()) {

	error_e e = reader.read();
	if (!is_ok(e)) {
	  return e;
	}

	builder.build(reader);
  }

  return error_ok;
}

inline error_e build_tree(tree_t* tree, const char* str)
{
  return build_tree(tree, str, strlen(str));
}

// for testing
inline tree_t build_tree(const char* str)
{
  tree_t t;
  error_e err = build_tree(&t, str);
  assert(is_ok(err));
  return t;
}

ONT_NAMESPACE_END

#endif//ONTREE_BUILDER_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
