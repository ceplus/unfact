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

#ifndef ONTREE_FIND_HPP
#define ONTREE_FIND_HPP

#include <ontree/tree.hpp>
#include <ontree/strings.hpp>
#include <string>
#include <cctype>

ONT_NAMESPACE_BEGIN

class finder_t
{
public:
  struct path_name_t { const char* str; size_t size; };
  struct path_t
  {
	path_t() : type(type_undefined) {}

	union
	{
	  path_name_t name;
	  size_t index;
	} u;

	type_e type;
	const_range_t left;
  };


  finder_t(node_t* here) : m_root(here) {}

  static path_t read_path(const_range_t& str)
  {
	path_t ret;

	if (0 == str.size()) {
	  return ret;
	} 

	if (str.at(0) == '.') {
	  str.consume(1);
	  if (0 == str.size()) {
		return ret;
	  } 
	}

	if (isalnum(str.at(0))) {
	  size_t i = 1;
	  for (/* */; i < str.size(); ++i) {
		if (!isalnum(str.at(i))) {
		  break;
		}
	  }

	  ret.type = type_object;
	  ret.u.name.str  = str.head();
	  ret.u.name.size = i;
	  ret.left = str.consumed(i);
	  return ret;
	}

	if (str.at(0) == '[') {
	  str.consume(1);
	  if (0 == str.size()) {
		return ret;
	  } 

	  size_t i = 0;
	  for (/* */; i < str.size(); ++i) {
		if (str.at(i) == ']') {
		  break;
		}

		if (!isdigit(str.at(i))) {
		  return ret;
		}
	  }

	  if (0 == i) {
		return ret;
	  }

	  char buf[64];
	  size_t len  = min_of(i, 64U);
	  memcpy(buf, str.head(), len);
	  buf[len] = '\0';
	  size_t num = size_t(atoi(buf));


	  ret.type = type_array;
	  ret.u.index = num;
	  ret.left = str.consumed(i+1); // + 1 for "]"
	  return ret;
			
	}

	ONT_NOT_IMPLEMENTED_YET();
		
	return ret;
  }

  static path_t read_path(const char* str) { return read_path(const_range_t(str)); }


  error_e find(const char* path, size_t len, node_t** found) const
  {
	const_range_t left = const_range_t(path, len);
	node_t* here = m_root;

	while (!left.empty()) {

	  if (!here) {
		return error_not_found;
	  }

	  path_t p = read_path(left);
	  switch (p.type) {
	  case type_object:
		if (here->type() == type_object) {
		  here = node_cast<object_t>(here)->find(const_range_t(p.u.name.str, p.u.name.size)).node();
		} else {
		  return error_not_found;
		}
		break;
	  case type_array:
		if (here->type() == type_array) {
		  here = node_cast<array_t>(here)->at(p.u.index).node();
		} else {
		  return error_not_found;
		}
		break;
	  case type_undefined:
		*found = 0;
		return error_ill_formed;
	  }

	  left = p.left;
	}

	*found = here;
	return error_ok;
  }

  error_e find(const char* path, node_t** found) const
  {
	return find(path, strlen(path), found);
  }

private:
  node_t* m_root;
};

ONT_NAMESPACE_END

#endif//ONTREE_FIND_HPP

/* -*-
   Local Variables:
   mode: c++
   c-tab-always-indent: t
   c-indent-level: 2
   c-basic-offset: 2
   End:
   -*- */
