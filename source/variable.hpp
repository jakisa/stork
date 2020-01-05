#ifndef variable_hpp
#define variable_hpp

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace stork {

	class variable;
	class number_variable;
	class string_variable;
	class array_variable;
	class function_variable;
	class reference_variable;

	using variable_ptr = std::shared_ptr<variable>;
	using number_variable_ptr = std::shared_ptr<number_variable>;
	using string_variable_ptr = std::shared_ptr<string_variable>;
	using array_variable_ptr = std::shared_ptr<array_variable>;
	using function_variable_ptr = std::shared_ptr<function_variable>;
	using reference_variable_ptr = std::shared_ptr<reference_variable>;

	class variable: public std::enable_shared_from_this<variable> {
	public:
		number_variable_ptr as_number();
		string_variable_ptr as_string();
		array_variable_ptr as_array();
		function_variable_ptr as_function();
		reference_variable_ptr as_reference();
	
		virtual ~variable();
	};

	class number_variable: public variable {
	public:
		double _value;
	};
	
	class string_variable: public variable {
	public:
		std::string _value;
	};
	
	using variable_ptr = std::shared_ptr<variable>;
	
	class array_variable: public variable {
	public:
		std::vector<variable_ptr> _value;
	};
	
	class runtime_context;
	
	class function_variable: public variable {
	public:
		std::function<void(runtime_context&)> _value;
	};
	
	class reference_variable: public variable {
	public:
		variable_ptr _value;
	};
}

#endif /* variable_hpp */
