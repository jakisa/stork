#include "expression.hpp"

namespace stork {
	void_expression::void_expression() {
	}
	
	void_expression::~void_expression(){
	}

	string_expression::string_expression() {
	}

	void string_expression::evaluate_void(runtime_context& context) const {
		evaluate_string(context);
	}
	
	number_expression::number_expression() {
	}

	std::string number_expression::evaluate_string(runtime_context& context) const {
		return std::to_string(evaluate_number(context));
	}

	void number_expression::evaluate_void(runtime_context& context) const {
		evaluate_number(context);
	}

	void evaluate(const void_expression_ptr& expr, runtime_context& context) {
		return expr->evaluate_void(context);
	}
	
	double evaluate(const number_expression_ptr& expr, runtime_context& context) {
		return expr->evaluate_number(context);
	}
	
	std::string evaluate(const string_expression_ptr& expr, runtime_context& context) {
		return expr->evaluate_string(context);
	}
	
	namespace {
		class variable_expression: public virtual void_expression {
		protected:
			variable_expression() {
			}
		public:
			virtual variable_ptr evaluate_variable(runtime_context& context) const = 0;
			
			void evaluate_void(runtime_context& context) const override {
				evaluate_variable(context);
			}
		};
		
		using variable_expression_ptr = std::unique_ptr<variable_expression>;
		
		class string_variable_expression: public variable_expression, public string_expression {
		public:
			void evaluate_void(runtime_context& context) const override {
				return variable_expression::evaluate_void(context);
			}
			
			string_variable_ptr evaluate_string_variable(runtime_context& context) const {
				return evaluate_variable(context)->as_string();
			}
			
			std::string evaluate_string(runtime_context& context) const override {
				return evaluate_string_variable(context)->value;
			}
		};
		
		using string_variable_expression_ptr = std::unique_ptr<string_variable_expression>;
		
		class number_variable_expression: public variable_expression, public number_expression {
		public:
			void evaluate_void(runtime_context& context) const override {
				return variable_expression::evaluate_void(context);
			}
			
			number_variable_ptr evaluate_number_variable(runtime_context& context) const {
				return evaluate_variable(context)->as_number();
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate_number_variable(context)->value;
			}
		};
		
		using number_variable_expression_ptr = std::unique_ptr<number_variable_expression>;
		
		variable_ptr evaluate(const variable_expression_ptr& expr, runtime_context& context) {
			return expr->evaluate_variable(context);
		}
		
		number_variable_ptr evaluate(const number_variable_expression_ptr& expr, runtime_context& context) {
			return expr->evaluate_number_variable(context);
		}
		
		string_variable_ptr evaluate(const string_variable_expression_ptr& expr, runtime_context& context) {
			return expr->evaluate_string_variable(context);
		}
		
		template<class base>
		class t_global_variable_expression: public base {
		private:
			int _idx;
		public:
			t_global_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				return context.global(_idx);
			}
		};
		
		using global_variable_expression = t_global_variable_expression<variable_expression>;
		using global_string_variable_expression = t_global_variable_expression<string_variable_expression>;
		using global_number_variable_expression = t_global_variable_expression<number_variable_expression>;
		
		template<class base>
		class t_local_variable_expression: public base {
		private:
			int _idx;
		public:
			t_local_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				return context.local(_idx);
			}
		};
		
		using local_variable_expression = t_local_variable_expression<variable_expression>;
		using local_string_variable_expression = t_local_variable_expression<string_variable_expression>;
		using local_number_variable_expression = t_local_variable_expression<number_variable_expression>;
		
		class preinc_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
		public:
			preinc_expression(number_variable_expression_ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
		
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr ret = evaluate(_expr1, context);
				++ret->value;
				return ret;
			}
		};
		
		class predec_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
		public:
			predec_expression(number_variable_expression_ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr ret = evaluate(_expr1, context);
				++ret->value;
				return ret;
			}
		};
		
		class postinc_expression: public number_expression {
		private:
			number_variable_expression_ptr _expr1;
		public:
			postinc_expression(number_variable_expression_ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
		
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context)->value++;
			}
		};
		
		class postdec_expression: public number_expression {
		private:
			number_variable_expression_ptr _expr1;
		public:
			postdec_expression(number_variable_expression_ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
		
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context)->value--;
			}
		};
		
		class positive_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
		public:
			positive_expression(number_expression_ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context);
			}
		};
		
		class negative_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
		public:
			negative_expression(number_expression_ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return -evaluate(_expr1, context);
			}
		};
		
		class add_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			add_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) + evaluate(_expr2, context);
			}
		};
		
		class sub_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			sub_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) - evaluate(_expr2, context);
			}
		};
		
		class concat_expression: public string_expression {
		private:
			string_expression_ptr _expr1;
			string_expression_ptr _expr2;
		public:
			concat_expression(string_expression_ptr expr1, string_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			std::string evaluate_string(runtime_context& context) const override {
				return evaluate(_expr1, context) + evaluate(_expr2, context);
			}
		};
		
		class mul_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			mul_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) * evaluate(_expr2, context);
			}
		};
		
		class div_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			div_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) / evaluate(_expr2, context);
			}
		};
		
		class idiv_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			idiv_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return int(evaluate(_expr1, context) / evaluate(_expr2, context));
			}
		};
		
		class mod_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			mod_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				double rslt1 = evaluate(_expr1, context);
				double rslt2 = evaluate(_expr2, context);
				return rslt1 - rslt1 * int(rslt1/rslt2);
			}
		};
		
		class bitnot_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
		public:
			bitnot_expression(number_expression_ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return ~int(evaluate(_expr1, context));
			}
		};
		
		class bitand_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			bitand_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return int(evaluate(_expr1, context)) & int(evaluate(_expr2, context));
			}
		};
		
		class bitor_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			bitor_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return int(evaluate(_expr1, context)) | int(evaluate(_expr2, context));
			}
		};
		
		class bitxor_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			bitxor_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return int(evaluate(_expr1, context)) ^ int(evaluate(_expr2, context));
			}
		};
		
		class bitsl_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			bitsl_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return int(evaluate(_expr1, context)) << int(evaluate(_expr2, context));
			}
		};
		
		class bitsr_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			bitsr_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return int(evaluate(_expr1, context)) >> int(evaluate(_expr2, context));
			}
		};
		
		class not_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
		public:
			not_expression(number_expression_ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return !evaluate(_expr1, context);
			}
		};
		
		class and_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			and_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) && evaluate(_expr2, context);
			}
		};
		
		class or_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			or_expression(number_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) || evaluate(_expr2, context);
			}
		};
		
		
		class assign_expression: public variable_expression {
		private:
			variable_expression_ptr _expr1;
			variable_expression_ptr _expr2;
		public:
			assign_expression(variable_expression_ptr expr1, variable_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				return evaluate(_expr1, context)->assign_from(evaluate(_expr2, context));
			}
		};
		
		class assign_string_expression: public string_variable_expression {
		private:
			string_variable_expression_ptr _expr1;
			string_expression_ptr _expr2;
		public:
			assign_string_expression(string_variable_expression_ptr expr1, string_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				return evaluate(_expr1, context)->assign_from(evaluate(_expr2, context));
			}
		};
		
		class assign_number_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			assign_number_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				return evaluate(_expr1, context)->assign_from(evaluate(_expr2, context));
			}
		};
		
		class add_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			add_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value += evaluate(_expr2, context);
				return rslt1;
			}
		};
		
		class sub_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			sub_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value -= evaluate(_expr2, context);
				return rslt1;
			}
		};
		
		class concat_assign_expression: public string_variable_expression {
		private:
			string_variable_expression_ptr _expr1;
			string_expression_ptr _expr2;
		public:
			concat_assign_expression(string_variable_expression_ptr expr1, string_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				string_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value += evaluate(_expr2, context);
				return rslt1;
			}
		};
		
		class mul_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			mul_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value *= evaluate(_expr2, context);
				return rslt1;
			}
		};
		
		class div_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			div_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value /= evaluate(_expr2, context);
				return rslt1;
			}
		};
		
		class idiv_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			idiv_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value = int(rslt1->value/evaluate(_expr2, context));
				return rslt1;
			}
		};
		
		class mod_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			mod_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				double rslt2 = evaluate(_expr2, context);
				rslt1->value -= rslt2 * int(rslt1->value/rslt2);
				return rslt1;
			}
		};
		
		class and_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			and_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value = int(rslt1->value) & int(evaluate(_expr2, context));
				return rslt1;
			}
		};
		
		class or_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			or_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value = int(rslt1->value) | int(evaluate(_expr2, context));
				return rslt1;
			}
		};
		
		class xor_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			xor_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value = int(rslt1->value) ^ int(evaluate(_expr2, context));
				return rslt1;
			}
		};
		
		class sl_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			sl_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value = int(rslt1->value) << int(evaluate(_expr2, context));
				return rslt1;
			}
		};
		
		class sr_assign_expression: public number_variable_expression {
		private:
			number_variable_expression_ptr _expr1;
			number_expression_ptr _expr2;
		public:
			sr_assign_expression(number_variable_expression_ptr expr1, number_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			variable_ptr evaluate_variable(runtime_context& context) const override {
				number_variable_ptr rslt1 = evaluate(_expr1, context);
				rslt1->value = int(rslt1->value) >> int(evaluate(_expr2, context));
				return rslt1;
			}
		};
		
		template <class expression_ptr>
		class t_eq_expression: public number_expression {
		private:
			expression_ptr _expr1;
			expression_ptr _expr2;
		public:
			t_eq_expression(expression_ptr expr1, expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) == evaluate(_expr2, context);
			}
		};
		
		template <class expression_ptr>
		class t_ne_expression: public number_expression {
		private:
			expression_ptr _expr1;
			expression_ptr _expr2;
		public:
			t_ne_expression(expression_ptr expr1, expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) != evaluate(_expr2, context);
			}
		};
		
		template <class expression_ptr>
		class t_lt_expression: public number_expression {
		private:
			expression_ptr _expr1;
			expression_ptr _expr2;
		public:
			t_lt_expression(expression_ptr expr1, expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) < evaluate(_expr2, context);
			}
		};
		
		template <class expression_ptr>
		class t_gt_expression: public number_expression {
		private:
			expression_ptr _expr1;
			expression_ptr _expr2;
		public:
			t_gt_expression(expression_ptr expr1, expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) > evaluate(_expr2, context);
			}
		};
		
		template <class expression_ptr>
		class t_le_expression: public number_expression {
		private:
			expression_ptr _expr1;
			expression_ptr _expr2;
		public:
			t_le_expression(expression_ptr expr1, expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) <= evaluate(_expr2, context);
			}
		};
		
		template <class expression_ptr>
		class t_ge_expression: public number_expression {
		private:
			expression_ptr _expr1;
			expression_ptr _expr2;
		public:
			t_ge_expression(expression_ptr expr1, expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) >= evaluate(_expr2, context);
			}
		};
		
		using eq_number_expression = t_eq_expression<number_expression_ptr>;
		using eq_string_expression = t_eq_expression<string_expression_ptr>;
		
		using ne_number_expression = t_ne_expression<number_expression_ptr>;
		using ne_string_expression = t_ne_expression<string_expression_ptr>;
		
		using lt_number_expression = t_lt_expression<number_expression_ptr>;
		using lt_string_expression = t_lt_expression<string_expression_ptr>;
		
		using gt_number_expression = t_gt_expression<number_expression_ptr>;
		using gt_string_expression = t_gt_expression<string_expression_ptr>;
		
		using le_number_expression = t_le_expression<number_expression_ptr>;
		using le_string_expression = t_le_expression<string_expression_ptr>;
		
		using ge_number_expression = t_ge_expression<number_expression_ptr>;
		using ge_string_expression = t_ge_expression<string_expression_ptr>;
		
		class ternary_number_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			number_expression_ptr _expr2;
			number_expression_ptr _expr3;
		public:
			ternary_number_expression(
				number_expression_ptr expr1,
				number_expression_ptr expr2,
				number_expression_ptr expr3
			) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2)),
				_expr3(std::move(expr3))
			{
			}
			
			double evaluate_number(runtime_context& context) const override {
				return evaluate(_expr1, context) ? evaluate(_expr2, context) : evaluate(_expr3, context);
			}
		};
		
		class ternary_string_expression: public number_expression {
		private:
			number_expression_ptr _expr1;
			string_expression_ptr _expr2;
			string_expression_ptr _expr3;
		public:
			ternary_string_expression(
				number_expression_ptr expr1,
				string_expression_ptr expr2,
				string_expression_ptr expr3
			) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2)),
				_expr3(std::move(expr3))
			{
			}
			
			std::string evaluate_string(runtime_context& context) const override {
				return evaluate(_expr1, context) ? evaluate(_expr2, context) : evaluate(_expr3, context);
			}
		};
	}
	/*
	{"?", reserved_token::question},
	
	{",", reserved_token::comma},
	
	{"(", reserved_token::open_round},
	{")", reserved_token::close_round},
	
	{"[", reserved_token::open_square},
	{"]", reserved_token::close_square},*/
}
