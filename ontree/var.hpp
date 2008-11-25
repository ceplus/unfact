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

#ifndef ONTREE_VAR_HPP
#define ONTREE_VAR_HPP

#include <ontree/base.hpp>
#include <ontree/tree.hpp>
#include <ontree/find.hpp>
#include <string>

ONT_NAMESPACE_BEGIN

class var_t
{
public:
  typedef var_t self_type;

  // keep implicit intentionaly
  var_t(node_t* node) : m_node(node) {}

  type_e type() const { return m_node ? m_node->type() : type_undefined; } 
  bool defined() const { return 0 != m_node; }
  bool string_p() const { return type() == type_string; }
  bool number_p() const { return type() == type_number; }

  var_t operator[](const char* str) const
  {
	if (m_node->type() != type_object) {
	  return 0;
	}

	object_t* self = node_cast<object_t>(m_node);
	member_iterator_t i = self->find(str);
	return (self->end() != i) ? i.node() : 0;
  }

  var_t operator[](size_t index) const
  {
	if (m_node->type() != type_array) {
	  return 0;
	}

	array_t* self = node_cast<array_t>(m_node);
	element_iterator_t i = self->at(index);
	return (self->end() != i) ? i.node() : 0;
  }

  const char* c_str() const
  {
	assert(type_string == type());
	return node_cast<string_t>(m_node)->value().c_str();
  }

  std::string to_s() const { return c_str(); }

  real_t to_f() const
  {
	assert(type_number == type());
	return node_cast<number_t>(m_node)->value();
  }

  object_t* to_object() const
  {
	assert(type_object == type());
	return node_cast<object_t>(m_node);
  }

  array_t* to_areray() const
  {
	assert(type_array == type());
	return node_cast<array_t>(m_node);
  }

  std::string name() const
  {
	assert(m_node && m_node->parent() && m_node->parent()->type() == type_object);
	return member_t::unnode(m_node)->name().c_str();
  }

  int    to_i() const { return static_cast<int>(to_f()); }
  size_t to_size() const { return static_cast<size_t>(to_f()); }

  bool to_p() const
  {
	assert(type_predicate == type());
	return node_cast<predicate_t>(m_node)->value();
  }

  bool null() const
  {
	return (type_null == type());
  }

  self_type& operator++() { increment(); return *this; }
  self_type  operator++(int) { self_type ret = *this; increment(); return ret; }

  var_t first() const 
  {
	switch (m_node->type()) {
	case type_object: {
	  member_iterator_t i = node_cast<object_t>(m_node)->begin();
	  if (i.child()) {
		return i.node();
	  } else {
		return 0;
	  }
	}   break;
	case type_array: {
	  element_iterator_t i = node_cast<array_t>(m_node)->begin();
	  if (i.child()) {
		return i.node();
	  } else {
		return 0;
	  }
	}   break;
	default:
	  assert(false);
	  return 0;
	}
  }

  size_t size() const 
  {
	switch (type()) {
	case type_object:
	  return node_cast<object_t>(m_node)->size();
	case type_array:
	  return node_cast<array_t>(m_node)->size();
	default:
	  assert(false);
	  return 0;
	}
  }

  var_t find(const char* path) const
  {
	node_t* found = 0;
	finder_t f(m_node);
	error_e err = f.find(path, &found);
	switch (err) {
	case error_ok:
	  return found;
	case error_ill_formed:
	  ONT_SHOULD_NOT_BE_REACHED();
	  return 0;
	default:
	  return 0;
	}
  }

  node_t* node() const { return m_node; }

public:
  void increment()
  {
	switch (m_node->parent()->type()) {
	case type_object: {
	  member_iterator_t i = member_t::unnode(m_node);
	  ++i;
	  if (i.child()) {
		m_node = i.node();
	  } else {
		m_node = 0;
	  }
	}   break;
	case type_array: {
	  element_iterator_t i = element_t::unnode(m_node);
	  ++i;
	  if (i.child()) {
		m_node = i.node();
	  } else {
		m_node = 0;
	  }
	}   break;
	default:
	  assert(false);
	  break;
	}
  }

private:
  node_t* m_node;
};

ONT_NAMESPACE_END

#endif//ONTREE_VAR_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
