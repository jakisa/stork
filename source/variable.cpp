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

	variable::~variable() {
	}
}

