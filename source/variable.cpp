#include "variable.hpp"

namespace stork {
	template<typename T>
	variable_impl<T>::variable_impl(T value):
		value(std::move(value))
	{
	}
	
	template<typename T>
	variable_ptr variable_impl<T>::clone() const {
		return std::make_shared<variable_impl<T> >(clone_variable_value(value));
	}
	
	template class variable_impl<number>;
	template class variable_impl<string>;
	template class variable_impl<function>;
	template class variable_impl<array>;
	
	number clone_variable_value(number value) {
		return value;
	}
	
	string clone_variable_value(const string& value) {
		return value;
	}
	
	function clone_variable_value(const function& value) {
		return value;
	}

	array clone_variable_value(const array& value) {
		array ret;
		for (const variable_ptr& v : value) {
			ret.push_back(v->clone());
		}
		return ret;
	}
}

