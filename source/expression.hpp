#ifndef expression_hpp
#define expression_hpp

#include "runtime_context.hpp"

#include <string>

namespace stork {
	class expression {
		expression(const expression&) = delete;
		void operator=(const expression&) = delete;
	protected:
		expression() {
		}
	public:
		virtual void evaluate(runtime_context& context) const = 0;
		virtual ~expression(){
		}
	};
	
	class string_expression : public expression {
	protected:
		string_expression() {
		}
	public:
		virtual std::string evaluate_string(runtime_context& context) const = 0;
		void evaluate(runtime_context& context) const override {
			evaluate_string(context);
		}
	};
	
	class lstring_expression : public string_expression {
	protected:
		lstring_expression() {
		}
	public:
		virtual std::string& evaluate_lstring(runtime_context& context) const = 0;
		std::string evaluate_string(runtime_context& context) const override {
			return evaluate_lstring(context);
		}
	};
	
	class number_expression : public string_expression {
	protected:
		number_expression() {
		}
	public:
		virtual double evaluate_number(runtime_context& context) const = 0;
		std::string evaluate_string(runtime_context& context) const override {
			return std::to_string(evaluate_number(context));
		}
		void evaluate(runtime_context& context) const override {
			evaluate_number(context);
		}
	};
	
	class lnumber_expression : public number_expression {
	protected:
		lnumber_expression() {
		}
	public:
		virtual double& evaluate_lnumber(runtime_context& context) const = 0;
		double evaluate_number(runtime_context& context) const override {
			return evaluate_lnumber(context);
		}
	};
}

#endif /* expression_hpp */
