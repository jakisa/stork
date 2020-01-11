#ifndef variable_hpp
#define variable_hpp

#include <memory>
#include <deque>
#include <functional>
#include <string>

namespace stork {

	class variable;
	
	using variable_ptr = std::shared_ptr<variable>;
	
	template <typename T>
	class variable_impl;
	
	class runtime_context;
	
	using number = double;
	using string = std::shared_ptr<std::string>;
	using array = std::deque<variable_ptr>;
	using function = std::shared_ptr<std::function<void(runtime_context&)> >;
	
	using number_variable = variable_impl<number>;
	using string_variable = variable_impl<string>;
	using array_variable = variable_impl<array>;
	using function_variable = variable_impl<function>;
	
	using number_variable_ptr = std::shared_ptr<number_variable>;
	using string_variable_ptr = std::shared_ptr<string_variable>;
	using array_variable_ptr = std::shared_ptr<array_variable>;
	using function_variable_ptr = std::shared_ptr<function_variable>;
	
	using number_variable_cptr = std::shared_ptr<const number_variable>;
	using string_variable_cptr = std::shared_ptr<const string_variable>;
	using array_variable_cptr = std::shared_ptr<const array_variable>;
	using function_variable_cptr = std::shared_ptr<function>;
	
	string to_string(number n);
	string to_string(number_variable_cptr v);
	
	class variable: public std::enable_shared_from_this<variable> {
	private:
		variable(const variable&) = delete;
		void operator=(const variable&) = delete;
	protected:
		variable() = default;
	public:
		virtual ~variable() = default;

		template <typename T>
		std::shared_ptr<variable_impl<T>> static_downcast() {
			return std::static_pointer_cast<variable_impl<T>>(shared_from_this());
		}
		
		template <typename T>
		std::shared_ptr<variable_impl<const T>> static_downcast() const {
			return std::static_pointer_cast<variable_impl<const T>>(shared_from_this());
		}
		
		template <typename T>
		T static_pointer_downcast() {
			return std::static_pointer_cast<typename T::element_type::value_type>(shared_from_this());
		}
		
		template <typename T>
		T static_pointer_downcast() const {
			return std::static_pointer_cast<const typename T::element_type::value_type>(shared_from_this());
		}
	};

	template<>
	class variable_impl<number>: public variable {
	public:
		using value_type = number;
		
		value_type value;
		
		variable_impl(value_type value);
	};

	template<>
	class variable_impl<string>: public variable {
	public:
		using value_type = string;
		
		value_type value;

		variable_impl(value_type value);
	};
	
	template<>
	class variable_impl<array>: public variable {
	public:
		using value_type = array;
		
		value_type value;

		variable_impl(value_type value);
	};
	
	template<>
	class variable_impl<function>: public variable {
	public:
		using value_type = function;
		
		value_type value;

		variable_impl(value_type value);
	};
}

#endif /* variable_hpp */
