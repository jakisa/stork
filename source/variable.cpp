#include "variable.hpp"

namespace stork {
	namespace {
		string from_std_string(std::string str) {
			return std::make_shared<std::string>(std::move(str));
		}
	}
	
	template<typename T>
	variable_impl<T>::variable_impl(T value):
		value(std::move(value))
	{
	}
	
	template<typename T>
	variable_ptr variable_impl<T>::clone() const {
		return std::make_shared<variable_impl<T> >(clone_variable_value(value));
	}
	
	template<typename T>
	string variable_impl<T>::to_string() const {
		return convert_to_string(value);
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
	
	string convert_to_string(number value) {
		if (value == int(value)) {
			return from_std_string(std::to_string(int(value)));
		} else {
			return from_std_string(std::to_string(value));
		}
	}
	
	string convert_to_string(const string& value) {
		return value;
	}
	
	string convert_to_string(const function& value) {
		return from_std_string("FUNCTION");
	}
	
	string convert_to_string(const array& value) {
		std::string ret = "[";
		const char* separator = "";
		for (const variable_ptr& v : value) {
			ret += separator;
			ret += *(v->to_string());
			separator = ", ";
		}
		ret += "]";
		return from_std_string(std::move(ret));
	}
	
	string convert_to_string(const lvalue& var) {
		return var->to_string();
	}
}

