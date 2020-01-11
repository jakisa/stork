#include "variable.hpp"

namespace stork {

	string to_string(number n) {
		return std::make_shared<std::string>(std::to_string(n));
	}
	
	string to_string(number_variable_cptr v) {
		return to_string(v->value);
	}

	number_variable::variable_impl(value_type value) :
		value(value)
	{
	}
	
	string_variable::variable_impl(value_type value) :
		value(std::move(value))
	{
	}

	array_variable::variable_impl(value_type value) :
		value(std::move(value))
	{
	}

	function_variable::variable_impl(value_type value) :
		value(std::move(value))
	{
	}
}

