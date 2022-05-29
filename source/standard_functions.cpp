#include "standard_functions.hpp"
#include "module.hpp"

#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace stork {

	void add_math_functions(stork_module& m) {
		m.add_external_function("sin", std::function<number(number)>(
			[](number x) {
				return std::sin(x);
			}
		));
		
		m.add_external_function("cos", std::function<number(number)>(
			[](number x) {
				return std::cos(x);
			}
		));
		
		m.add_external_function("tan", std::function<number(number)>(
			[](number x) {
				return std::tan(x);
			}
		));
		
		m.add_external_function("log", std::function<number(number)>(
			[](number x) {
				return std::log(x);
			}
		));
		
		m.add_external_function("exp", std::function<number(number)>(
			[](number x) {
				return std::exp(x);
			}
		));
		
		m.add_external_function("pow", std::function<number(number, number)>(
			[](number x, number y) {
				return std::pow(x, y);
			}
		));
		
		srand((unsigned int)time(0));
		
		m.add_external_function("rnd", std::function<number(number)>(
			[](number x) {
				return rand() % int(x);
			}
		));
	}
	
	void add_string_functions(stork_module& m) {
		m.add_external_function("strlen", std::function<number(const std::string&)>(
			[](const std::string& str) {
				return number(str.size());
			}
		));
		
		m.add_external_function("substr", std::function<std::string(const std::string&, number, number)>(
			[](const std::string& str, number from, number count) {
				return str.substr(size_t(from), size_t(count));
			}
		));
	}
	
	void add_trace_functions(stork_module& m) {
		m.add_external_function("trace", std::function<void(const std::string&)>(
			[](const std::string& str) {
				std::cout << str << std::endl;
			}
		));
	}
	
	void add_standard_functions(stork_module& m) {
		add_math_functions(m);
		add_string_functions(m);
		add_trace_functions(m);
	}

}
