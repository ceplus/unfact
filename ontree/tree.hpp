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

#ifndef ONTREE_TREE_HPP
#define ONTREE_TREE_HPP

#include <ontree/base.hpp>
#include <ontree/zone.hpp>
#include <ontree/text.hpp>
#include <cstring>
#include <algorithm>
#include <new>

ONT_NAMESPACE_BEGIN

enum type_e
{
  type_object,
  type_array,
  type_number,
  type_string,
  type_predicate,
  type_null,
  type_undefined, // node itsself is null: used by var_t
  types
};

class node_t
{
public:
  node_t(node_t* parent, type_e t)
	: m_parent(parent), m_type(t) {}
  ~node_t() { assert(!"should never be deleted!"); }
  type_e type() const { return m_type; }
  node_t* parent() const { return m_parent; }
private:
  node_t(const node_t&);
  const node_t& operator=(const node_t&);
private:
  node_t* m_parent;
  type_e m_type;
};

template<class T, type_e N>
class leaf_t : public node_t
{
public:
  typedef T value_type;
  leaf_t(node_t* parent, const value_type& val)
	: node_t(parent, N), m_value(val) {}
  value_type value() const { return m_value; }
  void set_value(value_type val) { m_value = val; }
private:
  value_type m_value;
};

typedef leaf_t<real_t, type_number>  number_t;
typedef leaf_t<text_t, type_string>  string_t;
typedef leaf_t<bool, type_predicate> predicate_t;
typedef leaf_t<none_t, type_null>    null_t;

template<class Child>
class child_iterator_t 
{
public:
  typedef Child child_type;
  typedef child_iterator_t self_type;
  typedef node_t value_type;

  child_iterator_t(child_type* child) : m_child(child) {}

  self_type& operator++() { increment(); return *this; }
  self_type  operator++(int) { self_type ret = *this; increment(); return ret; }
  value_type* operator->() const { return m_child->node(); }
  value_type& operator*() const { return *(m_child->node()); }

  text_t name() const { return m_child->name(); }
  value_type* node() const { return m_child ? m_child->node() : 0; }
  child_type* child() const { return m_child; }

  bool operator==(const child_iterator_t& that) const { return this->m_child == that.m_child; }
  bool operator!=(const child_iterator_t& that) const { return !(*this == that); }
private:
  void increment() { m_child = m_child->next(); }
private:
  child_type* m_child;
};

template<class Child>
class parent_t : public node_t
{
public:
  typedef Child child_type;
  typedef child_iterator_t<child_type> iterator_type;

  parent_t(node_t* parent, type_e t) : node_t(parent, t), m_first(0), m_last(0), m_size(0) {}

  iterator_type begin() const { return iterator_type(m_first); }
  iterator_type end() const { return iterator_type(0); }
  size_t size() const { return m_size; }

public: // used from tree_t
  void insert(child_type* child)
  {
	if (!m_first) {	m_first = child; }
	if (m_last) { m_last->set_next(child); }
	m_last = child;
	m_size++;
  }
private:
  child_type* m_first;
  child_type* m_last;
  size_t m_size;
};

class member_t
{
public:
  member_t(const text_t& name) : m_name(name), m_next(0) {}

  const text_t& name() const { return m_name; }
  member_t* next() const { return m_next; }
  void set_next(member_t* n) { m_next = n; }

  const node_t* node() const
  {
	return reinterpret_cast<const node_t*>
	  (reinterpret_cast<const byte_t*>(this) + sizeof(member_t));
  }

  node_t* node() 
  {
	assert(this);
	return reinterpret_cast<node_t*>
	  (reinterpret_cast<byte_t*>(this) + sizeof(member_t));
  }

  static member_t* unnode(node_t* node)
  {
	assert(node->parent()->type() == type_object);
	return reinterpret_cast<member_t*>
	  (reinterpret_cast<byte_t*>(node) - sizeof(member_t));		
  }

  type_e node_type() const { return node()->type(); }
	
private:
  text_t m_name;
  member_t* m_next;
};

typedef child_iterator_t<member_t> member_iterator_t;

class object_t : public parent_t<member_t>
{
public:
  typedef parent_t<member_t> base_type;
  typedef base_type::iterator_type iterator_type;

  explicit object_t(node_t* parent) : base_type(parent, type_object) {}

  iterator_type find(const const_range_t& name) const
  {
	for (iterator_type i=begin(); i != end(); ++i) {
	  if (i.name() == name) {
		return i;
	  }
	}
		
	return end();
  }

  iterator_type find(const char* name) const { return find(const_range_t(name, strlen(name))); }
};

class element_t
{
public:
  element_t() :  m_next(0) {}

  text_t name() const { return text_t(0, 0); } // for member_t compatility. almost useless.
  element_t* next() const { return m_next; }
  void set_next(element_t* n) { m_next = n; }

  const node_t* node() const
  {
	assert(this);
	return reinterpret_cast<const node_t*>
	  (reinterpret_cast<const byte_t*>(this) + sizeof(element_t));
  }

  node_t* node() 
  {
	return reinterpret_cast<node_t*>
	  (reinterpret_cast<byte_t*>(this) + sizeof(element_t));
  }

  static element_t* unnode(node_t* node)
  {
	assert(node->parent()->type() == type_array);
	return reinterpret_cast<element_t*>
	  (reinterpret_cast<byte_t*>(node) - sizeof(element_t));
  }

  type_e node_type() const { return node()->type(); }

private:
  element_t* m_next;
};

typedef child_iterator_t<element_t> element_iterator_t;

class array_t : public parent_t<element_t>
{
public:
  typedef parent_t<element_t> base_type;
  typedef base_type::iterator_type iterator_type;

  explicit array_t(node_t* parent) : base_type(parent, type_array) {}

  iterator_type at(size_t n) const
  {
	size_t off = 0;
	for (iterator_type i=begin(); i != end(); ++i, ++off) {
	  if (off == n) {
		return i;
	  }
	}
		
	return end();
  }
};

/*
 * node type tricks
 */
template<class T> inline type_e node_type();
template<> inline type_e node_type<number_t>() { return type_number; }
template<> inline type_e node_type<string_t>() { return type_string; }
template<> inline type_e node_type<predicate_t>() { return type_predicate; }
template<> inline type_e node_type<null_t>() { return type_null; }
template<> inline type_e node_type<object_t>() { return type_object; }
template<> inline type_e node_type<array_t>() { return type_array; }

template<class T, class ChildIterator>
inline T* child_cast(ChildIterator i)
{
  assert(node_type<T>() == i->type());
  return static_cast<T*>(i.node());
}

template<class T>
inline T* node_cast(node_t* node)
{
  assert(node_type<T>() == node->type());
  return static_cast<T*>(node);
}

class tree_t
{
public:
  explicit tree_t(size_t page_size=zone_t::default_page_size)
	: m_zone(page_size), m_root(0), m_size(1) {}

  tree_t(const tree_t& that)
	: m_zone(that.page_size()), m_root(0), m_size(1)
  {
	insert_children(root(), that.root());
  }

  explicit tree_t(const tree_t& that, const object_t* that_child)
	: m_zone(that.page_size()), m_root(0), m_size(1)
  {
	insert_children(root(), that_child);
  }

  const tree_t& operator=(const tree_t& that)
  {
	if (this != &that) {
	  tree_t copy(that);
	  copy.swap(*this);
	}

	return *this;
  }


  size_t size() const { return m_size; }

  object_t* root() const
  {
	if (!m_root) {
	  m_root = new (const_cast<tree_t*>(this)->m_zone.allocate(sizeof(object_t))) object_t(0);
	}

	return m_root;
  }
	
  number_t* insert_leaf(object_t* parent, const char* name, real_t val)
  {
	return insert_leaf(parent, new_text(name), val);
  }

  number_t* insert_leaf(object_t* parent, const text_t& name, real_t val)
  {
	return new (insert_member(parent, name, sizeof(number_t))->node()) number_t(parent, val);
  }

  number_t* insert_leaf(array_t* parent, real_t val)
  {
	return new (insert_element(parent, sizeof(number_t))->node())
	  number_t(parent, val);

  }

  string_t* insert_leaf(object_t* parent, const char* name, const char* value)
  {
	return insert_leaf(parent, new_text(name), new_text(value));
  }

  string_t* insert_leaf(object_t* parent, const text_t& name, const text_t& value)
  {
	return new (insert_member(parent, name, sizeof(string_t))->node()) string_t(parent, value);
  }

  string_t* insert_leaf(array_t* parent, const text_t& value)
  {
	return new 
	  (insert_element(parent, sizeof(string_t))->node())
	  string_t(parent, value);
  }

  string_t* insert_leaf(array_t* parent, const char* value)
  {
	return insert_leaf(parent, new_text(value));
  }

  predicate_t* insert_leaf(object_t* parent, const char* name, bool val)
  {
	return insert_leaf(parent, new_text(name), val);
  }

  predicate_t* insert_leaf(object_t* parent, const text_t& name, bool val)
  {
	return new (insert_member(parent, name, sizeof(predicate_t))->node())
	  predicate_t(parent, val);
  }

  predicate_t* insert_leaf(array_t* parent, bool val)
  {
	return new (insert_element(parent, sizeof(predicate_t))->node())
	  predicate_t(parent, val);
  }

  null_t* insert_leaf(object_t* parent, const char* name, const none_t& none)
  {
	return insert_leaf(parent, new_text(name), none);
  }

  null_t* insert_leaf(object_t* parent, const text_t& name, const none_t&)
  {
	return new (insert_member(parent, name, sizeof(null_t))->node()) null_t(parent, none_t());
  }

  null_t* insert_leaf(array_t* parent, const none_t&)
  {
	return new (insert_element(parent, sizeof(null_t))->node()) null_t(parent, none_t());
  }

  object_t* insert_object(object_t* parent, const char* name)
  {
	return insert_object(parent, new_text(name));
  }

  object_t* insert_object(object_t* parent, const text_t& name)
  {
	return new (insert_member(parent, name, sizeof(object_t))->node()) object_t(parent);
  }

  object_t* insert_object(array_t* parent)
  {
	return new (insert_element(parent, sizeof(object_t))->node()) object_t(parent);
  }

  array_t* insert_array(object_t* parent, const char* name)
  {
	return insert_array(parent, new_text(name));
  }

  array_t* insert_array(object_t* parent, const text_t& name)
  {
	return new (insert_member(parent, name, sizeof(array_t))->node()) array_t(parent);
  }

  array_t* insert_array(array_t* parent)
  {
	return new (insert_element(parent, sizeof(array_t))->node()) array_t(parent);
  }

  text_t new_text(size_t size)
  {
	char* s = reinterpret_cast<char*>(m_zone.allocate(size+1));
	return text_t(s, size);
  }

  text_t new_text(const char* str)
  {
	size_t len = strlen(str);
	text_t ret = new_text(len);
	memcpy(ret.data(), str, len);
	ret.data()[len] = '\0';
	return ret;
  }

  void swap(tree_t& that)
  {
	std::swap(this->m_root, that.m_root);
	std::swap(this->m_size, that.m_size);
	this->m_zone.swap(that.m_zone);
  }

  size_t page_size() const { return m_zone.page_size(); }

  void insert_subtree(object_t* dst_parent, const char* name, node_t* src)
  {
	switch (src->type()) {
	case type_object: {
	  object_t* dst = insert_object(dst_parent, name);
	  for (member_iterator_t i = node_cast<object_t>(src)->begin(), end = node_cast<object_t>(src)->end();
		   i != end; ++i) {
		insert_subtree(dst, i.name().c_str(), i.node());
	  } 
	}   break;
	case type_array: {
	  array_t* dst = insert_array(dst_parent, name);
	  for (element_iterator_t i = node_cast<array_t>(src)->begin(), end = node_cast<array_t>(src)->end();
		   i != end; ++i) {
		insert_subtree(dst, i.node());
	  } 
	}   break;
	case type_string:
	  // make copy of text_t instead of share storage instance...
	  insert_leaf(dst_parent, new_text(name), 
				  new_text(node_cast<string_t>(src)->value().c_str()));
	  break;
	case type_number:
	  insert_leaf(dst_parent, new_text(name), node_cast<number_t>(src)->value());
	  break;
	case type_predicate:
	  insert_leaf(dst_parent, new_text(name), node_cast<predicate_t>(src)->value());
	  break;
	case type_null:
	  insert_leaf(dst_parent, new_text(name), none_t());
	  break;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  break;
	}
  }

  void insert_subtree(array_t* dst_parent, node_t* src)
  {
	switch (src->type()) {
	case type_object: {
	  object_t* dst = insert_object(dst_parent);
	  for (member_iterator_t i = node_cast<object_t>(src)->begin(), end = node_cast<object_t>(src)->end();
		   i != end; ++i) {
		insert_subtree(dst, i.name().c_str(), i.node());
	  } 
	}   break;
	case type_array: {
	  array_t* dst = insert_array(dst_parent);
	  for (element_iterator_t i = node_cast<array_t>(src)->begin(), end = node_cast<array_t>(src)->end();
		   i != end; ++i) {
		insert_subtree(dst, i.node());
	  } 
	}   break;
	case type_string:
	  insert_leaf(dst_parent, node_cast<string_t>(src)->value());
	  break;
	case type_number:
	  insert_leaf(dst_parent, node_cast<number_t>(src)->value());
	  break;
	case type_predicate:
	  insert_leaf(dst_parent, node_cast<predicate_t>(src)->value());
	  break;
	case type_null:
	  insert_leaf(dst_parent, none_t());
	  break;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  break;
	}
  }

public: // for test
  void insert_children(object_t* src_parent, const object_t* dst_parent)
  {
	for (member_iterator_t i = dst_parent->begin();
		 i != dst_parent->end(); ++i) {
	  insert_subtree(src_parent, i.name().c_str(), i.node());
	}
  }

private:
  member_t* insert_member(object_t* parent, const text_t& name, size_t node_size)
  {
	member_t* m = new (m_zone.allocate(sizeof(member_t) + node_size)) member_t(name);
	// XXX: check duplication
	parent->insert(m);
	m_size++;
	return m;
  }

  element_t* insert_element(array_t* parent, size_t node_size)
  {
	element_t* e
	  = new (m_zone.allocate(sizeof(element_t) + node_size)) element_t();
	parent->insert(e);
	m_size++;
	return e;
  }

private:
  zone_t m_zone;
  mutable object_t* m_root;
  size_t m_size;
};


ONT_NAMESPACE_END

#endif//ONTREE_TREE_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
