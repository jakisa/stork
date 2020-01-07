#include "expression.hpp"

namespace stork {
	namespace {
		template<class VariablePtr>
		struct variable_traits {
			using rvalue = VariablePtr;
		};
		
		template<>
		struct variable_traits<string_variable_ptr> {
			using rvalue = std::string;
		};
		
		template<>
		struct variable_traits<number_variable_ptr> {
			using rvalue = double;
		};
	
		template<class ExpressionPtr>
		class void_expression_wrapper: public void_expression {
		private:
			ExpressionPtr _expr1;
		public:
			void_expression_wrapper(ExpressionPtr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			void evaluate(runtime_context& context) const override {
				_expr1->evaluate(context);
			}
		};
		
		class string_expression_wrapper: public string_expression {
		private:
			number_expression::ptr _expr1;
		public:
			string_expression_wrapper(number_expression::ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			std::string evaluate(runtime_context& context) const override {
				return std::to_string(_expr1->evaluate(context));
			}
		};
		
		template<typename Value>
		class unbox_epresssion: public expression<Value> {
		private:
			using boxed_expression_ptr = typename expression<typename variable_impl<Value>::ptr>::ptr;
			boxed_expression_ptr _expr1;
		public:
			unbox_epresssion(boxed_expression_ptr expr1)
				: _expr1(std::move(expr1))
			{
			}
			
			Value evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context).value;
			}
		};

		template<class VariablePtr>
		class global_variable_expression: public expression<VariablePtr> {
		private:
			int _idx;
		public:
			global_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			VariablePtr evaluate(runtime_context& context) const override {
				return context.global(_idx)->template static_pointer_downcast<VariablePtr>();
			}
		};

		template<class VariablePtr>
		class local_variable_expression: public expression<VariablePtr> {
		private:
			int _idx;
		public:
			local_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			VariablePtr evaluate(runtime_context& context) const override {
				return context.global(_idx)->template static_pointer_downcast<VariablePtr>();
			}
		};
		
		class preinc_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
		public:
			preinc_expression(number_variable_expression::ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
		
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr ret = _expr1->evaluate(context);
				++ret->value;
				return ret;
			}
		};
		
		class predec_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
		public:
			predec_expression(number_variable_expression::ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
		
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr ret = _expr1->evaluate(context);
				--ret->value;
				return ret;
			}
		};
		
		class postinc_expression: public number_expression {
		private:
			number_variable_expression::ptr _expr1;
		public:
			postinc_expression(number_variable_expression::ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
		
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context)->value++;
			}
		};
	
		class postdec_expression: public number_expression {
		private:
			number_variable_expression::ptr _expr1;
		public:
			postdec_expression(number_variable_expression::ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
		
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context)->value--;
			}
		};
		
		class positive_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
		public:
			positive_expression(number_expression::ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context);
			}
		};
		
		class negative_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
		public:
			negative_expression(number_expression::ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return -_expr1->evaluate(context);
			}
		};
		
		template<class ValueT>
		class add_expression: public expression<ValueT> {
		private:
			using expression_ptr = typename expression<ValueT>::ptr;
			expression_ptr _expr1;
			expression_ptr _expr2;
		public:
			add_expression(expression_ptr expr1, expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			ValueT evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) + _expr2->evaluate(context);
			}
		};
		
		class sub_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			sub_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) - _expr2->evaluate(context);
			}
		};
		
		class mul_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			mul_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) * _expr2->evaluate(context);
			}
		};
		
		class div_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			div_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) / _expr2->evaluate(context);
			}
		};
		
		class idiv_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			idiv_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return int(_expr1->evaluate(context) / _expr2->evaluate(context));
			}
		};
		
		class mod_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			mod_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				double rslt1 = _expr1->evaluate(context);
				double rslt2 = _expr2->evaluate(context);
				return rslt1 - rslt1 * int(rslt1/rslt2);
			}
		};
		
		class bitnot_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
		public:
			bitnot_expression(number_expression::ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			double evaluate(runtime_context& context) const {
				return ~int(_expr1->evaluate(context));
			}
		};
		
		class bitand_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			bitand_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return int(_expr1->evaluate(context)) & int(_expr2->evaluate(context));
			}
		};
		
		class bitor_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			bitor_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return int(_expr1->evaluate(context)) | int(_expr2->evaluate(context));
			}
		};
		
		class bitxor_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			bitxor_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return int(_expr1->evaluate(context)) ^ int(_expr2->evaluate(context));
			}
		};
		
		class bitsl_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			bitsl_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return int(_expr1->evaluate(context)) << int(_expr2->evaluate(context));
			}
		};
		
		class bitsr_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			bitsr_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return int(_expr1->evaluate(context)) >> int(_expr2->evaluate(context));
			}
		};
		
		class not_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
		public:
			not_expression(number_expression::ptr expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			double evaluate(runtime_context& context) const {
				return !_expr1->evaluate(context);
			}
		};
		
		class and_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			and_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) && _expr2->evaluate(context);
			}
		};
		
		class or_expression: public number_expression {
		private:
			number_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			or_expression(number_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) && _expr2->evaluate(context);
			}
		};
		
		template<class VariablePtr>
		class assign_expression: public expression<VariablePtr> {
		private:
			using lvalue_expression_ptr = typename expression<VariablePtr>::ptr;
			using rvalue_expression_ptr = expression<typename variable_traits<VariablePtr>::rvalue>;
			lvalue_expression_ptr _expr1;
			rvalue_expression_ptr _expr2;
			
			assign_expression(lvalue_expression_ptr expr1, rvalue_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			VariablePtr evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context)->assign_from(_expr2->evaluate(context));
			}
		};
		
		template<class VariablePtr>
		class add_assign_expression: public expression<VariablePtr> {
		private:
			using lvalue_expression_ptr = typename expression<VariablePtr>::ptr;
			using rvalue_expression_ptr = expression<typename variable_traits<VariablePtr>::rvalue>;
			lvalue_expression_ptr _expr1;
			rvalue_expression_ptr _expr2;
			
			add_assign_expression(lvalue_expression_ptr expr1, rvalue_expression_ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			VariablePtr evaluate(runtime_context& context) const override {
				VariablePtr rslt1 = _expr1->evaluate(context);
				rslt1->value += _expr2->evaluate(context);
				return rslt1;
			}
		};
		
		class sub_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			sub_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value -= _expr2->evaluate(context);
				return rslt1;
			}
		};
		
		class mul_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			mul_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value *= _expr2->evaluate(context);
				return rslt1;
			}
		};
		
		class div_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			div_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value /= _expr2->evaluate(context);
				return rslt1;
			}
		};
		
		class idiv_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			idiv_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value/_expr2->evaluate(context));
				return rslt1;
			}
		};
		
		class mod_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			mod_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				double rslt2 = _expr2->evaluate(context);
				rslt1->value -= rslt2 * int(rslt1->value/rslt2);
				return rslt1;
			}
		};
		
		class and_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			and_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) & int(_expr2->evaluate(context));
				return rslt1;
			}
		};
		
		class or_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			or_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) | int(_expr2->evaluate(context));
				return rslt1;
			}
		};

		class xor_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			xor_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) ^ int(_expr2->evaluate(context));
				return rslt1;
			}
		};

		class sl_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			sl_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) << int(_expr2->evaluate(context));
				return rslt1;
			}
		};

		class sr_assign_expression: public number_variable_expression {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			sr_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			number_variable_ptr evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) >> int(_expr2->evaluate(context));
				return rslt1;
			}
		};
		
		template <class ExpressionPtr>
		class eq_expression: public number_expression {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			eq_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) == _expr2->evaluate(context);
			}
		};
		
		template <class ExpressionPtr>
		class ne_expression: public number_expression {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			ne_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) != _expr2->evaluate(context);
			}
		};
		
		template <class ExpressionPtr>
		class lt_expression: public number_expression {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			lt_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) < _expr2->evaluate(context);
			}
		};
		
		template <class ExpressionPtr>
		class gt_expression: public number_expression {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			gt_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) > _expr2->evaluate(context);
			}
		};
		
		template <class ExpressionPtr>
		class le_expression: public number_expression {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			le_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) <= _expr2->evaluate(context);
			}
		};
		
		template <class ExpressionPtr>
		class ge_expression: public number_expression {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			ge_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			double evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) >= _expr2->evaluate(context);
			}
		};
		
		template<typename T>
		class ternary_expression: public expression<T> {
		private:
			number_expression::ptr _expr1;
			typename expression<T>::ptr _expr2;
			typename expression<T>::ptr _expr3;
		public:
			ternary_expression(
				number_expression::ptr expr1,
				typename expression<T>::ptr expr2,
				typename expression<T>::ptr expr3
			) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2)),
				_expr3(std::move(expr3))
			{
			}
			
			T evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context) ? _expr2->evaluate(context) : _expr3->evaluate(context);
			}
		};
		
		template <typename T>
		class comma_expression: public expression<T> {
		private:
			void_expression::ptr _expr1;
			typename expression<T>::ptr _expr2;
		public:
			comma_expression(void_expression::ptr expr1, typename expression<T>::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			T evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context), _expr2->evaluate(context);
			}
		};
	}
	/*
	{"(", reserved_token::open_round},
	{")", reserved_token::close_round},
	
	{"[", reserved_token::open_square},
	{"]", reserved_token::close_square},*/
}
