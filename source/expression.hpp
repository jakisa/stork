#ifndef expression_hpp
#define expression_hpp

#include "runtime_context.hpp"

#include <string>

namespace stork {
	class void_expression {
		void_expression(const void_expression&) = delete;
		void operator=(const void_expression&) = delete;
	protected:
		void_expression();
	public:
		virtual void evaluate_void(runtime_context& context) const = 0;
		virtual ~void_expression();
	};
	
	using void_expression_ptr = std::unique_ptr<void_expression>;
	
	class string_expression: public virtual void_expression {
	protected:
		string_expression();
	public:
		virtual std::string evaluate_string(runtime_context& context) const = 0;
		void evaluate_void(runtime_context& context) const override;
	};
	
	using string_expression_ptr = std::unique_ptr<string_expression>;
	
	class number_expression: public string_expression {
	protected:
		number_expression();
	public:
		virtual double evaluate_number(runtime_context& context) const = 0;
		std::string evaluate_string(runtime_context& context) const override;
		void evaluate_void(runtime_context& context) const override;
	};
	
	using number_expression_ptr = std::unique_ptr<number_expression>;

	void evaluate(const void_expression_ptr& expr, runtime_context& context);
	double evaluate(const number_expression_ptr& expr, runtime_context& context);
	std::string evaluate(const string_expression_ptr& expr, runtime_context& context);
}

#endif /* expression_hpp */
