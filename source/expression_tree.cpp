#include "expression_tree.hpp"

namespace stork {

	node::node(node_value value, std::vector<node_ptr> children, size_t line_number, size_t char_index) :
		_value(std::move(value)),
		_children(std::move(children)),
		_line_number(line_number),
		_char_index(char_index)
	{
	}
		
	bool node::is_node_operation() const {
		return std::holds_alternative<node_operation>(_value);
	}
	
	bool node::is_identifier() const {
		return std::holds_alternative<identifier>(_value);
	}
	
	bool node::is_number() const {
		return std::holds_alternative<double>(_value);
	}
	
	bool node::is_string() const {
		return std::holds_alternative<std::string>(_value);
	}
	
	node_operation node::get_node_operation() const {
		return std::get<node_operation>(_value);
	}
	
	std::string_view node::get_identifier() const {
		return std::get<identifier>(_value).name;
	}
	
	double node::get_number() const {
		return std::get<double>(_value);
	}
	
	std::string_view node::get_string() const {
		return std::get<std::string>(_value);
	}
	
	const std::vector<node_ptr>& node::children() const {
		return _children;
	}
	
	size_t node::line_number() const {
		return _line_number;
	}
	
	size_t node::char_index() const {
		return _char_index;
	}
}
