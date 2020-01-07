#ifndef variable_hpp
#define variable_hpp

#include <memory>
#include <string>
#include <deque>
#include <functional>

namespace stork {

	class variable;
	class number_variable;
	class string_variable;
	class array_variable;
	class function_variable;

	using variable_ptr = std::shared_ptr<variable>;
	using number_variable_ptr = std::shared_ptr<number_variable>;
	using string_variable_ptr = std::shared_ptr<string_variable>;
	using array_variable_ptr = std::shared_ptr<array_variable>;
	using function_variable_ptr = std::shared_ptr<function_variable>;
	
	using number_variable_cptr = std::shared_ptr<const number_variable>;
	using string_variable_cptr = std::shared_ptr<const string_variable>;
	using array_variable_cptr = std::shared_ptr<const array_variable>;
	using function_variable_cptr = std::shared_ptr<const function_variable>;
	
	class variable: public std::enable_shared_from_this<variable> {
	public:
		number_variable_ptr as_number();
		string_variable_ptr as_string();
		array_variable_ptr as_array();
		function_variable_ptr as_function();
		
		number_variable_cptr as_number() const;
		string_variable_cptr as_string() const;
		array_variable_cptr as_array() const;
		function_variable_cptr as_function() const;
		
		virtual ~variable();

		virtual variable_ptr clone() const = 0;
		virtual variable_ptr assign_from(const variable_ptr& rhs) = 0;
	};

	class number_variable: public variable {
	public:
		double value;
		
		number_variable(double value);

		variable_ptr clone() const override;
		variable_ptr assign_from(const variable_ptr& rhs) override;
		variable_ptr assign_from(double value);
	};
	
	class string_variable: public variable {
	public:
		std::string value;

		string_variable(std::string value);

		variable_ptr clone() const override;
		variable_ptr assign_from(const variable_ptr& rhs) override;
		variable_ptr assign_from(std::string value);
	};
	
	class array_variable: public variable {
	public:
		std::deque<variable_ptr> value;

		array_variable(std::deque<variable_ptr> value);

		variable_ptr clone() const override;
		variable_ptr assign_from(const variable_ptr& rhs) override;
	};
	
	class runtime_context;
	
	class function_variable: public variable {
	public:
		std::function<void(runtime_context&)> value;

		function_variable(std::function<void(runtime_context&)> value);

		variable_ptr clone() const override;
		variable_ptr assign_from(const variable_ptr& rhs) override;
	};
}

#endif /* variable_hpp */
