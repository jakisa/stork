#include "expression.hpp"

namespace stork {
	namespace {
		template<typename To, typename From>
		struct converter {
			const To& operator()(const From& from) const {
				return from->value;
			}
		};
		
		template<typename T>
		struct converter<T, T> {
			const T& operator()(const T& t) const {
				return t;
			}
		};
		
		template<typename From>
		struct converter<void, From> {
			void operator()(const From&) const {
			}
		};
		
		template<typename From>
		struct converter<string, From> {
			string operator()(const From& from) const {
				return to_string(from);
			}
		};
		
		template<typename To, typename From>
		To convert(From&& from) {
			return converter<To, From>()(std::forward(from));
		}
	
		template<typename T, class V>
		class global_variable_expression: public expression<T> {
		private:
			int _idx;
		public:
			global_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			T evaluate(runtime_context& context) const override {
				return convert<T>(
					std::move(context.global(_idx)->template static_pointer_downcast<V>())
				);
			}
		};
		
		template<typename T, class V>
		class local_variable_expression: public expression<T> {
		private:
			int _idx;
		public:
			local_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			T evaluate(runtime_context& context) const override {
				return convert<T>(
					std::move(context.local(_idx)->template static_pointer_downcast<V>())
				);
			}
		};
		
		template<typename T, class C>
		class constant_expression: public expression<T> {
		private:
			T _c;
		public:
			constant_expression(C c) :
				_c(convert<T>(std::move(c)))
			{
			}
			
			T evaluate(runtime_context& context) const override {
				return _c;
			}
		};
		
		template<typename R, typename T1, class O>
		class unary_expression: public expression<R> {
		private:
			using Expr1 = typename expression<T1>::ptr;
			Expr1 _expr1;
		public:
			unary_expression(Expr1 expr1) :
				_expr1(std::move(expr1))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(O()(_expr1->evaluate(context)));
			}
		};
		
		template<typename R, typename T1, typename T2, class O>
		class binary_expression: public expression<R> {
		private:
			using Expr1 = typename expression<T1>::ptr;
			using Expr2 = typename expression<T1>::ptr;
			Expr1 _expr1;
			Expr2 _expr2;
		public:
			binary_expression(Expr1 expr1, Expr2 expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(O()(_expr1->evaluate(context), _expr2->evaluate(context)));
			}
		};

#define UNARY_EXPRESSION(name, RT, T1, code)\
		struct name {\
			RT operator()(T1 t1) {\
				code;\
			}\
		};\
		template<typename R>\
		using name##_expression = unary_expression<R, T1, name>;
		
		UNARY_EXPRESSION(preinc, number_variable_ptr, number_variable_ptr,
			++t1->value;
			return t1;
		);
		
		UNARY_EXPRESSION(predec, number_variable_ptr, number_variable_ptr,
			--t1->value;
			return t1;
		);

		UNARY_EXPRESSION(postinc, number, number_variable_ptr,
			return t1->value++;
		);
		
		UNARY_EXPRESSION(postdec, number, number_variable_ptr,
			return t1->value--;
		);
		
		UNARY_EXPRESSION(positive, number, number,
			return t1;
		);
		
		UNARY_EXPRESSION(negative, number, number,
			return -t1;
		);
		
		UNARY_EXPRESSION(bnot, number, number,
			return ~int(t1);
		);
		
		UNARY_EXPRESSION(lnot, number, number,
			return !t1;
		);

#undef UNARY_EXPRESSION

#define BINARY_EXPRESSION(name, RT, T1, T2, code)\
		struct name {\
			RT operator()(T1 t1, T2 t2) {\
				code;\
			}\
		};\
		template<typename R>\
		using name##_expression = binary_expression<R, T1, T2, name>;

		BINARY_EXPRESSION(add, number, number, number,
			return t1 + t2;
		);
		
		BINARY_EXPRESSION(sub, number, number, number,
			return t1 - t2;
		);
		
		BINARY_EXPRESSION(mul, number, number, number,
			return t1 * t2;
		);
		
		BINARY_EXPRESSION(div, number, number, number,
			return t1 / t2;
		);
		
		BINARY_EXPRESSION(idiv, number, number, number,
			return int(t1 / t2);
		);
		
		BINARY_EXPRESSION(mod, number, number, number,
			return t1 - t2 * int(t1/t2);
		);
		
		BINARY_EXPRESSION(band, number, number, number,
			return int(t1) & int(t2);
		);
		
		BINARY_EXPRESSION(bor, number, number, number,
			return int(t1) | int(t2);
		);
		
		BINARY_EXPRESSION(bxor, number, number, number,
			return int(t1) ^ int(t2);
		);
		
		BINARY_EXPRESSION(sl, number, number, number,
			return int(t1) << int(t2);
		);
		
		BINARY_EXPRESSION(sr, number, number, number,
			return int(t1) >> int(t2);
		);
		
		BINARY_EXPRESSION(concat, string, string, string,
			return t1 + t2;
		);
#undef BINARY_EXPRESSION

		/*
		struct assign {
			number_variable_ptr operator()(number_variable_ptr t1, number t2) const {
				t1->value = t2;
				return t1;
			}
			
			string_variable_ptr operator()(string_variable_ptr t1, string t2) const {
				t1->value = std::move(t2);
				return t1;
			}
			
			function_variable_ptr operator()(function_variable_ptr t1, function_variable_ptr t2) const {
				t1->value = t2->value;
				return t1;
			}
		};
		*/
		/*
		template<typename Ret, class VariablePtr>
		class assign_expression: public expression<Ret> {
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
			
			Ret evaluate(runtime_context& context) const override {
				return convert<Ret>(_expr1->evaluate(context)->assign_from(_expr2->evaluate(context)));
			}
		};
		
		template<typename Ret, class VariablePtr>
		class add_assign_expression: public expression<Ret> {
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
			
			Ret evaluate(runtime_context& context) const override {
				VariablePtr rslt1 = _expr1->evaluate(context);
				rslt1->value += _expr2->evaluate(context);
				return convert<Ret>(std::move(rslt1));
			}
		};
		
		template<typename Ret>
		class sub_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			sub_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value -= _expr2->evaluate(context);
				return convert<Ret>(std::move(rslt1));
			}
		};
		
		template<typename Ret>
		class mul_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			mul_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value *= _expr2->evaluate(context);
				return convert<Ret>(std::move(rslt1));
			}
		};
		
		template<typename Ret>
		class div_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			div_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value /= _expr2->evaluate(context);
				return convert<Ret>(std::move(rslt1));
			}
		};
		
		template<typename Ret>
		class idiv_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			idiv_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value/_expr2->evaluate(context));
				return convert<Ret>(std::move(rslt1));
			}
		};
		
		template<typename Ret>
		class mod_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			mod_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				number rslt2 = _expr2->evaluate(context);
				rslt1->value -= rslt2 * int(rslt1->value/rslt2);
				return convert<Ret>(std::move(rslt1));
			}
		};
		
		template<typename Ret>
		class and_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			and_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) & int(_expr2->evaluate(context));
				return convert<Ret>(std::move(rslt1));
			}
		};
		
		template<typename Ret>
		class or_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			or_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) | int(_expr2->evaluate(context));
				return convert<Ret>(std::move(rslt1));
			}
		};

		template<typename Ret>
		class xor_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			xor_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) ^ int(_expr2->evaluate(context));
				return convert<Ret>(std::move(rslt1));
			}
		};

		template<typename Ret>
		class sl_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			sl_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) << int(_expr2->evaluate(context));
				return convert<Ret>(std::move(rslt1));
			}
		};

		template<typename Ret>
		class sr_assign_expression: public expression<Ret> {
		private:
			number_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
			
			sr_assign_expression(number_variable_expression::ptr expr1, number_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				number_variable_ptr rslt1 = _expr1->evaluate(context);
				rslt1->value = int(rslt1->value) >> int(_expr2->evaluate(context));
				return convert<Ret>(std::move(rslt1));
			}
		};
		
		template <typename Ret, class ExpressionPtr>
		class eq_expression: public expression<Ret> {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			eq_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				return convert<Ret>(_expr1->evaluate(context) == _expr2->evaluate(context));
			}
		};
		
		template <typename Ret, class ExpressionPtr>
		class ne_expression: public expression<Ret> {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			ne_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				return convert<Ret>(_expr1->evaluate(context) != _expr2->evaluate(context));
			}
		};
		
		template <typename Ret, class ExpressionPtr>
		class lt_expression: public expression<Ret> {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			lt_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				return convert<Ret>(_expr1->evaluate(context) < _expr2->evaluate(context));
			}
		};
		
		template <typename Ret, class ExpressionPtr>
		class gt_expression: public expression<Ret> {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			gt_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				return convert<Ret>(_expr1->evaluate(context) > _expr2->evaluate(context));
			}
		};
		
		template <typename Ret, class ExpressionPtr>
		class le_expression: public expression<Ret> {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			le_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				return convert<Ret>(_expr1->evaluate(context) <= _expr2->evaluate(context));
			}
		};
		
		template <typename Ret, class ExpressionPtr>
		class ge_expression: public expression<Ret> {
		private:
			ExpressionPtr _expr1;
			ExpressionPtr _expr2;
		public:
			ge_expression(ExpressionPtr expr1, ExpressionPtr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				return convert<Ret>(_expr1->evaluate(context) >= _expr2->evaluate(context));
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
		
		template <typename Ret, typename S, typename T>
		class comma_expression: public expression<Ret> {
		private:
			typename expression<T>::ptr _expr1;
			typename expression<S>::ptr _expr2;
		public:
			comma_expression(typename expression<T>::ptr expr1, typename expression<S>::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			Ret evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context), convert<Ret>(_expr2->evaluate(context));
			}
		};
		
		template <typename T>
		class array_expression: public expression<T> {
		private:
			array_variable_expression::ptr _expr1;
			number_variable_expression::ptr _expr2;
		public:
			array_expression(array_variable_expression::ptr expr1, number_variable_expression::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			T evaluate(runtime_context& context) const override {
				return _expr1->evaluate(context)->value[_expr2->evaluate(context)];
			}
		};
		
		*/
		template<typename R>
		class land_expression: public expression<R> {
		private:
			expression<number>::ptr _expr1;
			expression<number>::ptr _expr2;
		public:
			land_expression(expression<number>::ptr expr1, expression<number>::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(_expr1->evaluate(context) && _expr2->evaluate(context));
			}
		};
		
		template<typename R>
		class lor_expression: public expression<R> {
		private:
			expression<number>::ptr _expr1;
			expression<number>::ptr _expr2;
		public:
			lor_expression(expression<number>::ptr expr1, expression<number>::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(_expr1->evaluate(context) || _expr2->evaluate(context));
			}
		};
	}
/*
	{"(", reserved_token::open_round},
	{")", reserved_token::close_round},
*/
}
