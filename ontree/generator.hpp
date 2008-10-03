
#ifndef ONTREE_GENERATOR_HPP
#define ONTREE_GENERATOR_HPP

#include <ontree/base.hpp>
#include <ontree/tree.hpp>
#include <ontree/event.hpp>
#include <ontree/writer.hpp>
#include <string>

ONT_NAMESPACE_BEGIN

class generator_t
{
public:
  generator_t(const tree_t* tree) : m_tree(tree), m_node(0), m_last(event_begin) {}

  void next()
  {
	switch (m_last) {
	case event_begin:
	  m_node = m_tree->root();
	  m_last = event_object_begin;
	  break;
	case event_end:
	  ONT_SHOULD_NOT_BE_REACHED();
	  break;
	case event_object_begin:
	  if (0 < node_cast<object_t>(m_node)->size()) {
		m_node = node_cast<object_t>(m_node)->begin().node();
		m_last = event_object_key;
	  } else {
		m_last = event_object_end;
	  }
	  break;
	case event_object_key:
	  m_last = to_event(m_node->type());
	  break;
	case event_array_begin:
	  if (0 < node_cast<array_t>(m_node)->size()) {
		m_node = node_cast<array_t>(m_node)->begin().node();
		m_last = to_event(m_node->type());
	  } else {
		m_last = event_array_end;
	  }
	  break;
	case event_object_end:
	case event_array_end:
	case event_number:
	case event_string:
	case event_predicate:
	case event_null:
	  sibling_or_parent();
	  break;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  break;
	}
  }

  event_e last() const { return m_last; }
  bool done() const { return m_last == event_end; } // TODO: impl

  const char* key() const
  {
	assert(event_object_key == m_last);
	return member_t::unnode(m_node)->name().c_str();
  }

  const char* string() const
  {
	assert(event_string == m_last);
	return node_cast<string_t>(m_node)->value().c_str();
  }

  real_t number() const
  {
	assert(event_number == m_last);
	return node_cast<number_t>(m_node)->value();
  }
	
  bool predicate() const
  {
	assert(event_predicate == m_last);
	return node_cast<predicate_t>(m_node)->value();
  }
	
public:

  void sibling_or_parent()
  {
	if (!m_node->parent()) { // we've on the root: finish traversal
	  m_node = 0;
	  m_last = event_end;
	  return;
	} else {
	  event_e next_event = event_end;
	  switch (m_node->parent()->type()) {
	  case type_object: {
		member_t* next_member = member_t::unnode(m_node)->next();
		if (next_member) {
		  m_node = next_member->node();
		  m_last = event_object_key;
		} else {
		  m_node = m_node->parent();
		  m_last = event_object_end; 
		}
	  }   break;
	  case type_array: {
		element_t* next_element = element_t::unnode(m_node)->next();
		if (next_element) {
		  m_node = next_element->node();
		  m_last = to_event(m_node->type());
		} else {
		  m_node = m_node->parent();
		  m_last = event_array_end; 
		} 

	  }   break;
	  default:
		ONT_SHOULD_NOT_BE_REACHED();
		return;
	  }
	}
  }

  static event_e to_event(type_e type)
  {
	switch (type)
	{
	case type_object:
	  return event_object_begin;
	case type_array:
	  return event_array_begin;
	case type_string:
	  return event_string;
	case type_number:
	  return event_number;
	case type_predicate:
	  return event_predicate;
	case type_null:
	  return event_null;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  return event_end;
	}
  }
	
private:
  const tree_t* m_tree;
  node_t* m_node;
  event_e m_last;
};

inline std::string generate_string(const tree_t& tree, size_t initial_size=128)
{
  std::string ret(initial_size, ' ');
  generator_t gen(&tree);
  writer_t writer;
  writer.set_buffer(&(ret[0]), ret.size());
  error_e err = error_ok;
  do {
	switch (gen.last()) {
	case event_object_begin:
	  err = writer.write_begin(scope_object);
	  break;
	case event_object_end:
	  err = writer.write_end();
	  break;
	case event_object_key:
	  err = writer.write_key(gen.key());
	  break;
	case event_array_begin:
	  err = writer.write_begin(scope_array);
	  break;
	case event_array_end:
	  err = writer.write_end();
	  break;
	case event_number:
	  err = writer.write_value(gen.number());
	  break;
	case event_string:
	  err = writer.write_value(gen.string());
	  break;
	case event_predicate:
	  err = writer.write_value(gen.predicate());
	  break;
	case event_null:
	  err = writer.write_value(none_t());
	  break;
	case event_begin:
	case event_end:
	  break;
	default:
	  ONT_SHOULD_NOT_BE_REACHED();
	  return "";
	}

	switch (err)
	{
	case error_ok:
	  gen.next();
	  break;
	case error_need_buffer: {
	  size_t written = writer.buffer().head() - &(ret[0]);
	  ret.resize(ret.size()*2);
	  writer.set_buffer(&(ret[written]), ret.size() - written);
	}   break;
	default:
	  assert(false);
	  return "";
	}
  } while (!gen.done());

  ret.resize(writer.buffer().head() - &(ret[0]));
  return ret;
}


ONT_NAMESPACE_END

#endif//ONTREE_GENERATOR_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
