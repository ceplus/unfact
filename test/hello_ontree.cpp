
#include <ontree/builder.hpp>
#include <ontree/var.hpp>
#include <string>
#include <fstream>

using namespace ontree;

void hello_ontree()
{
	// read from file
	std::ifstream in("ydn.json");
	in.seekg (0, std::ios::end);
	std::streamsize len = in.tellg();
	in.seekg (0, std::ios::beg);
	std::string str(len, ' ');
	in.read(&(str[0]), len);
	 
	// now let's build the tree!
	tree_t t;
	error_e err = build_tree(&t, str.c_str());
	assert(is_ok(err));

	// hash access 
	var_t root = t.root();
	assert("229307" == root["ResultSet"]["totalResultsAvailable"].to_s());

	// array access
	var_t results = root["ResultSet"]["Result"];
	assert("425" == results[0U]["Width"].to_s());
	assert("473" == results[1U]["Width"].to_s());

	// iteration
	var_t iter = root["ResultSet"]["Result"].first();
	assert("425" == iter["Width"].to_s());
	iter++;
	assert("473" == iter["Width"].to_s());
	iter++;
	assert(!iter.defined());

	// finder
	assert("http://scd.mm-b1.yimg.com/image/500892420" == root.find("ResultSet.Result[1].Thumbnail.Url").to_s());
}
