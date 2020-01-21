#include "expression_tree.hpp"
#include "helpers.hpp"
#include "errors.hpp"

namespace stork {
	namespace {
		bool is_convertible(type_handle type_from, bool lvalue_from, type_handle type_to, bool lvalue_to) {
			if (type_to == type_registry::get_void_handle()) {
				return true;
			}
			if (lvalue_to) {
				return lvalue_from && type_from == type_to;
			}
			if (type_from == type_to) {
				return true;
			}
			return type_from == type_registry::get_number_handle() && type_to == type_registry::get_string_handle();
		}
	}

	node::node(compiler_context& context, node_value value, std::vector<node_ptr> children, size_t line_number, size_t char_index) :
		_value(std::move(value)),
		_children(std::move(children)),
		_line_number(line_number),
		_char_index(char_index)
	{
		const type_handle void_handle = type_registry::get_void_handle();
		const type_handle number_handle = type_registry::get_number_handle();
		const type_handle string_handle = type_registry::get_string_handle();
		
		std::visit(overloaded {
			[&](const std::string& value) {
				_type_id = string_handle;
				_lvalue = false;
			},
			[&](double value) {
				_type_id = number_handle;
				_lvalue = false;
			},
			[&](const identifier& value){
				if (const identifier_info* info = context.find(value.name)) {
					_type_id = info->type_id();
					_lvalue = !info->is_constant();
				} else {
					throw undeclared_error(value.name, _line_number, _char_index);
				}
			},
			[&](node_operation value){
				switch (value) {
					case node_operation::param:
						_type_id = _children[0]->_type_id;
						_lvalue = false;
						break;
					case node_operation::preinc:
					case node_operation::predec:
						_type_id = number_handle;
						_lvalue = true;
						_children[0]->check_conversion(number_handle, true);
						break;
					case node_operation::postinc:
					case node_operation::postdec:
						_type_id = number_handle;
						_lvalue = false;
						_children[0]->check_conversion(number_handle, true);
						break;
					case node_operation::positive:
					case node_operation::negative:
					case node_operation::bnot:
					case node_operation::lnot:
						_type_id = number_handle;
						_lvalue = false;
						_children[0]->check_conversion(number_handle, false);
						break;
					case node_operation::add:
					case node_operation::sub:
					case node_operation::mul:
					case node_operation::div:
					case node_operation::idiv:
					case node_operation::mod:
					case node_operation::band:
					case node_operation::bor:
					case node_operation::bxor:
					case node_operation::bsl:
					case node_operation::bsr:
					case node_operation::land:
					case node_operation::lor:
						_type_id = number_handle;
						_lvalue = false;
						_children[0]->check_conversion(number_handle, false);
						_children[1]->check_conversion(number_handle, false);
						break;
					case node_operation::eq:
					case node_operation::ne:
					case node_operation::lt:
					case node_operation::gt:
					case node_operation::le:
					case node_operation::ge:
						_type_id = number_handle;
						_lvalue = false;
						if (!_children[0]->is_number() || !_children[1]->is_number()) {
							_children[0]->check_conversion(string_handle, false);
							_children[1]->check_conversion(string_handle, false);
						} else {
							_children[0]->check_conversion(number_handle, false);
							_children[1]->check_conversion(number_handle, false);
						}
						break;
					case node_operation::concat:
						_type_id = context.get_handle(simple_type::string);
						_lvalue = false;
						_children[0]->check_conversion(string_handle, false);
						_children[1]->check_conversion(string_handle, false);
						break;
					case node_operation::assign:
						_type_id = _children[0]->get_type_id();
						_lvalue = true;
						_children[0]->check_conversion(_type_id, true);
						_children[1]->check_conversion(_type_id, false);
						break;
					case node_operation::add_assign:
					case node_operation::sub_assign:
					case node_operation::mul_assign:
					case node_operation::div_assign:
					case node_operation::idiv_assign:
					case node_operation::mod_assign:
					case node_operation::band_assign:
					case node_operation::bor_assign:
					case node_operation::bxor_assign:
					case node_operation::bsl_assign:
					case node_operation::bsr_assign:
						_type_id = number_handle;
						_lvalue = true;
						_children[0]->check_conversion(number_handle, true);
						_children[1]->check_conversion(number_handle, false);
						break;
					case node_operation::concat_assign:
						_type_id = string_handle;
						_lvalue = true;
						_children[0]->check_conversion(string_handle, true);
						_children[1]->check_conversion(string_handle, false);
						break;
					case node_operation::comma:
						for (int i = 0; i < int(_children.size()) - 1; ++i) {
							_children[i]->check_conversion(void_handle, false);
						}
						_type_id = _children.back()->get_type_id();
						_lvalue = _children.back()->is_lvalue();
						break;
					case node_operation::index:
						if (const array_type* at = std::get_if<array_type>(_children[0]->get_type_id())) {
							_type_id = at->inner_type_id;
							_lvalue = _children[0]->is_lvalue(); 
						} else {
							throw semantic_error(to_string(_children[0]->_type_id) + " is not indexable",
							                     _line_number, _char_index);
						}
						break;
					case node_operation::ternary:
						_children[0]->check_conversion(number_handle, false);
						if (is_convertible(
							_children[2]->get_type_id(), _children[2]->is_lvalue(),
							_children[1]->get_type_id(), _children[1]->is_lvalue()
						)) {
							_children[2]->check_conversion(_children[1]->get_type_id(), _children[1]->is_lvalue());
							_type_id = _children[1]->get_type_id();
							_lvalue = _children[1]->is_lvalue();
						} else {
							_children[1]->check_conversion(_children[2]->get_type_id(), _children[2]->is_lvalue());
							_type_id = _children[2]->get_type_id();
							_lvalue = _children[2]->is_lvalue();
						}
						break;
					case node_operation::call:
						if (const function_type* ft = std::get_if<function_type>(_children[0]->get_type_id())) {
							_type_id = ft->return_type_id;
							_lvalue = false;
							if (ft->param_type_id.size() + 1 != _children.size()) {
								throw semantic_error("Wrong number of arguments. "
								                     "Expected " + std::to_string(ft->param_type_id.size()) +
								                     ", given " + std::to_string(_children.size() - 1),
								                     _line_number, _char_index);
							}
							for (size_t i = 0; i < ft->param_type_id.size(); ++i) {
								if (_children[i+1]->is_lvalue() && !ft->param_type_id[i].by_ref) {
									throw semantic_error(
										"Function doesn't receive the argument by reference",
										_children[i+1]->get_line_number(), _children[i+1]->get_char_index()
									);
								}
								_children[i+1]->check_conversion(ft->param_type_id[i].type_id, ft->param_type_id[i].by_ref);
							}
						} else {
							throw semantic_error(to_string(_children[0]->_type_id) + " is not callable",
							                     _line_number, _char_index);
						}
						break;
				}
			}
		},_value);
		
		_type_id_original = _type_id;
		_lvalue_original = _lvalue;
	}
	
	const node_value& node::get_value() const {
		return _value;
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
	
	const std::vector<node_ptr>& node::get_children() const {
		return _children;
	}
	
	type_handle node::get_type_id() const {
		return _type_id;
	}
	
	bool node::is_lvalue() const {
		return _lvalue;
	}
	
	type_handle node::get_type_id_original() const {
		return _type_id_original;
	}

	bool node::is_lvalue_original() const {
		return _lvalue_original;
	}
	
	size_t node::get_line_number() const {
		return _line_number;
	}
	
	size_t node::get_char_index() const {
		return _char_index;
	}
	
	void node::check_conversion(type_handle type_id, bool lvalue) const{
		if (!is_convertible(_type_id, _lvalue, type_id, lvalue)) {
			throw wrong_type_error(std::to_string(_type_id), std::to_string(type_id), lvalue,
			                       _line_number, _char_index);
		}
	}
}
