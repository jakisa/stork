#include "variable.hpp"

namespace stork {

	number_variable_ptr variable::as_number() {
		return std::static_pointer_cast<number_variable>(shared_from_this());
	}
	
	string_variable_ptr variable::as_string() {
		return std::static_pointer_cast<string_variable>(shared_from_this());
	}
	
	array_variable_ptr variable::as_array() {
		return std::static_pointer_cast<array_variable>(shared_from_this());
	}
	
	function_variable_ptr variable::as_function() {
		return std::static_pointer_cast<function_variable>(shared_from_this());
	}
	
	number_variable_cptr variable::as_number() const{
		return std::static_pointer_cast<const number_variable>(shared_from_this());
	}
	
	string_variable_cptr variable::as_string() const{
		return std::static_pointer_cast<const string_variable>(shared_from_this());
	}
	
	array_variable_cptr variable::as_array() const{
		return std::static_pointer_cast<const array_variable>(shared_from_this());
	}
	
	function_variable_cptr variable::as_function() const{
		return std::static_pointer_cast<const function_variable>(shared_from_this());
	}

	variable::~variable() {
	}
	
	number_variable::number_variable(double value) :
		value(value)
	{
	}
	
	variable_ptr number_variable::clone() const {
		return std::make_shared<number_variable>(value);
	}
	
	variable_ptr number_variable::assign_from(const variable_ptr& rhs) {
		return assign_from(rhs->as_number()->value);
	}
	
	variable_ptr number_variable::assign_from(double value) {
		this->value = value;
		return shared_from_this();
	}
	
	string_variable::string_variable(std::string value) :
		value(std::move(value))
	{
	}

	variable_ptr string_variable::clone() const {
		return std::make_shared<string_variable>(value);
	}
	
	variable_ptr string_variable::assign_from(const variable_ptr& rhs) {
		return assign_from(rhs->as_string()->value);
	};
	
	variable_ptr string_variable::assign_from(std::string value) {
		this->value = std::move(value);
		return shared_from_this();
	};
	
	array_variable::array_variable(std::deque<variable_ptr> value) :
		value(std::move(value))
	{
	}

	variable_ptr array_variable::clone() const {
		std::deque<variable_ptr> cloned_value;
		for (const variable_ptr& v : value) {
			cloned_value.push_back(v->clone());
		}
		return std::make_shared<array_variable>(std::move(cloned_value));
	}
	
	variable_ptr array_variable::assign_from(const variable_ptr& rhs) {
		value.clear();
		
		for (const variable_ptr& v : rhs->as_array()->value) {
			value.push_back(v->clone());
		}
		
		return shared_from_this();
	}
	
	function_variable::function_variable(std::function<void(runtime_context&)> value) :
		value(std::move(value))
	{
	}
	
	variable_ptr function_variable::clone() const {
		return std::make_shared<function_variable>(value);
	}
	
	variable_ptr function_variable::assign_from(const variable_ptr& rhs) {
		value = rhs->as_function()->value;
		
		return shared_from_this();
	}
}

