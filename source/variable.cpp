#include "variable.hpp"

namespace stork {

	number_variable::variable_impl(value_type value) :
		value(value)
	{
	}
	
	variable_ptr number_variable::clone() const {
		return std::make_shared<number_variable>(value);
	}
	
	variable_ptr number_variable::assign_from(const variable_ptr& rhs) {
		return assign_from(rhs->static_downcast<value_type>()->value);
	}
	
	variable_ptr number_variable::assign_from(value_type value) {
		this->value = value;
		return shared_from_this();
	}
	
	string_variable::variable_impl(std::string value) :
		value(std::move(value))
	{
	}

	variable_ptr string_variable::clone() const {
		return std::make_shared<string_variable>(value);
	}
	
	variable_ptr string_variable::assign_from(const variable_ptr& rhs) {
		return assign_from(rhs->static_downcast<value_type>()->value);
	};
	
	variable_ptr string_variable::assign_from(std::string value) {
		this->value = std::move(value);
		return shared_from_this();
	};
	
	array_variable::variable_impl(value_type value) :
		value(std::move(value))
	{
	}

	variable_ptr array_variable::clone() const {
		value_type cloned_value;
		for (const variable_ptr& v : value) {
			cloned_value.push_back(v->clone());
		}
		return std::make_shared<array_variable>(std::move(cloned_value));
	}
	
	variable_ptr array_variable::assign_from(const variable_ptr& rhs) {
		value.clear();
		
		for (const variable_ptr& v : rhs->static_downcast<value_type>()->value) {
			value.push_back(v->clone());
		}
		
		return shared_from_this();
	}
	
	function_variable::variable_impl(value_type value) :
		value(std::move(value))
	{
	}
	
	variable_ptr function_variable::clone() const {
		return std::make_shared<function_variable>(value);
	}
	
	variable_ptr function_variable::assign_from(const variable_ptr& rhs) {
		value = rhs->static_downcast<value_type>()->value;
		
		return shared_from_this();
	}
}

