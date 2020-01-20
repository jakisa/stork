#include "variable.hpp"

namespace stork {
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

