#ifndef variable_hpp
#define variable_hpp

#include <memory>
#include <string>
#include <deque>
#include <functional>

namespace stork {

	class variable;
	
	using variable_ptr = std::shared_ptr<variable>;
	
	template <typename T>
	class variable_impl;
	
	class runtime_context;
	
	using number_variable = variable_impl<double>;
	using string_variable = variable_impl<std::string>;
	using array_variable = variable_impl<std::deque<variable_ptr> >;
	using function_variable = variable_impl<std::function<void(runtime_context&)> >;
	
	using number_variable_ptr = std::shared_ptr<number_variable>;
	using string_variable_ptr = std::shared_ptr<string_variable>;
	using array_variable_ptr = std::shared_ptr<array_variable>;
	using function_variable_ptr = std::shared_ptr<function_variable>;
	
	using number_variable_cptr = std::shared_ptr<const number_variable>;
	using string_variable_cptr = std::shared_ptr<const string_variable>;
	using array_variable_cptr = std::shared_ptr<const array_variable>;
	using function_variable_cptr = std::shared_ptr<const function_variable>;
	
	class variable: public std::enable_shared_from_this<variable> {
	private:
		variable(const variable&) = delete;
		void operator=(const variable&) = delete;
	protected:
		variable() = default;
	public:
		virtual ~variable() = default;

		virtual variable_ptr clone() const = 0;
		virtual variable_ptr assign_from(const variable_ptr& rhs) = 0;
		
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
	class variable_impl<double>: public variable {
	public:
		using value_type = double;
		
		value_type value;
		
		variable_impl(value_type value);

		variable_ptr clone() const override;
		variable_ptr assign_from(const variable_ptr& rhs) override;
		variable_ptr assign_from(double value);
	};

	template<>
	class variable_impl<std::string>: public variable {
	public:
		using value_type = std::string;
		
		value_type value;

		variable_impl(value_type value);

		variable_ptr clone() const override;
		variable_ptr assign_from(const variable_ptr& rhs) override;
		variable_ptr assign_from(std::string value);
	};
	
	template<>
	class variable_impl<std::deque<variable_ptr> >: public variable {
	public:
		using value_type = std::deque<variable_ptr>;
		
		value_type value;

		variable_impl(value_type value);

		variable_ptr clone() const override;
		variable_ptr assign_from(const variable_ptr& rhs) override;
	};
	
	template<>
	class variable_impl<std::function<void(runtime_context&)> >: public variable {
	public:
		using value_type = std::function<void(runtime_context&)>;
		
		value_type value;

		variable_impl(value_type value);

		variable_ptr clone() const override;
		variable_ptr assign_from(const variable_ptr& rhs) override;
	};
}

#endif /* variable_hpp */
