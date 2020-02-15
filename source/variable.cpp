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
	
	template<typename T>
	std::string variable_impl<T>::to_string() const {
		if constexpr(std::is_same<number, T>::value) {
			if (value == int(value)) {
				return std::to_string(int(value));
			} else {
				return std::to_string(value);
			}
		} else if constexpr(std::is_same<string, T>::value) {
			return *value;
		} else if constexpr(std::is_same<function, T>::value) {
			return "FUNCTION";
		} else {
			std::string ret = "[";
			const char* separator = "";
			for (const variable_ptr& v : value) {
				ret += separator;
				ret += v->to_string();
				separator = ", ";
			}
			ret += "]";
			return ret;
		}
	}
	
	template class variable_impl<number>;
	template class variable_impl<string>;
	template class variable_impl<function>;
	template class variable_impl<array>;
	template class variable_impl<tuple>;
	
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
	
	tuple clone_variable_value(const tuple& value) {
		tuple ret;
		for (const variable_ptr& v : value) {
			ret.push_back(v->clone());
		}
		return ret;
	}
}

