#include "expression.hpp"
#include <type_traits>

namespace stork {
	namespace {
		template<typename T>
		struct variable_traits {
			using rvalue = T;
		};
		
		template<>
		struct variable_traits<number_variable_ptr> {
			using rvalue = number;
		};
		
		template<>
		struct variable_traits<string_variable_ptr> {
			using rvalue = string;
		};
	
		template<typename To, typename From>
		struct converter;
	
		template<typename To>
		struct converter<To, std::shared_ptr<variable_impl<To> > > {
			const To& operator()(const std::shared_ptr<variable_impl<To> >& from) const {
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
		
		template<>
		struct converter<string, string> {
			const string& operator()(const string& from) const {
				return from;
			}
		};
		
		template<typename To, typename From,
			std::enable_if_t<std::is_same<number, From>::value || !std::is_arithmetic<From>::value, int> = 0
		>
		To convert(From&& from) {
			return converter<To, From>()(std::forward<From>(from));
		}
		
		template<typename To, typename From,
			std::enable_if_t<!std::is_same<number, From>::value && std::is_arithmetic<From>::value, int> = 0
		>
		To convert(const From& from) {
			return convert<To>(number(from));
		}
		
		/*
		Removed while work is in progress, to disable unused function warnings.
		
		number_variable_ptr assign(number_variable_ptr n1, number n2) {
			n1->value = n2;
			return n1;
		}
		
		string_variable_ptr assign(string_variable_ptr s1, string s2) {
			s1->value = std::move(s2);
			return s1;
		}
		
		function_variable_ptr assign(function_variable_ptr t1, function_variable_ptr t2) {
			t1->value = t2->value;
			return t1;
		}
		
		number lt(number n1, number n2) {
			return n1 < n2;
		}
		
		number lt(string s1, string s2) {
			return *s1 < *s2;
		}
		
		*/
	
		template<typename T>
		class global_variable_expression: public expression<T> {
		private:
			int _idx;
		public:
			global_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			T evaluate(runtime_context& context) const override {
				return context.global(_idx)->template static_pointer_downcast<T>();
			}
		};
		
		template<typename T>
		class local_variable_expression: public expression<T> {
		private:
			int _idx;
		public:
			local_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			T evaluate(runtime_context& context) const override {
				return context.local(_idx)->template static_pointer_downcast<T>();
			}
		};
		
		template<typename T>
		class constant_expression: public expression<const T&> {
		private:
			T _c;
		public:
			constant_expression(T c) :
				_c(std::move(c))
			{
			}
			
			const T& evaluate(runtime_context& context) const override {
				return _c;
			}
		};
		/*
		template<typename R, typename T1, typename T2, class O>
		class binary_expression: public expression<R> {
		private:
			using Expr1 = typename expression<T1>::ptr;
			using Expr2 = typename expression<T2>::ptr;
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
		 */
		template<class O, typename R, typename... Ts>
		class generic_expression: public expression<R> {
		private:
			std::tuple<typename expression<Ts>::ptr...> _exprs;
			
			template<typename... Exprs>
			R evaluate_tuple(runtime_context& context, const Exprs&... exprs) const {
				return convert<R>(O()(exprs->evaluate(context)...));
			}
		public:
			generic_expression(typename expression<Ts>::ptr... exprs) :
				_exprs(std::move(exprs)...)
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return std::apply(
					[&](const auto&... exprs){
						return this->evaluate_tuple(context, exprs...);
					},
					_exprs
				);
			}
		};

#define UNARY_EXPRESSION(name, code)\
		struct name##_op {\
			template <typename T1> \
			auto operator()(T1 t1) {\
				code;\
			}\
		};\
		template<typename R, typename T1>\
		using name##_expression = generic_expression<name##_op, R, T1>;

#define BINARY_EXPRESSION(name, code)\
		struct name##_op {\
			template <typename T1, typename T2>\
			auto operator()(T1 t1, T2 t2) {\
				code;\
			}\
		};\
		template<typename R, typename T1, typename T2>\
		using name##_expression = generic_expression<name##_op, R, T1, T2>;

#define TERNARY_EXPRESSION(name, code)\
		struct name##_op {\
			template <typename T1, typename T2, typename T3>\
			auto operator()(T1 t1, T2 t2, T3 t3) {\
				code;\
			}\
		};\
		template<typename R, typename T1, typename T2, typename T3>\
		using name##_expression = generic_expression<name##_op, R, T1, T2, T3>;


		UNARY_EXPRESSION(preinc,
			++t1->value;
			return t1;
		);
		
		UNARY_EXPRESSION(predec,
			--t1->value;
			return t1;
		);

		UNARY_EXPRESSION(postinc, return t1->value++);
		
		UNARY_EXPRESSION(postdec, return t1->value--);
		
		UNARY_EXPRESSION(positive, return t1);
		
		UNARY_EXPRESSION(negative, return -t1);
		
		UNARY_EXPRESSION(bnot, return ~int(t1));
		
		UNARY_EXPRESSION(lnot, return !t1);

		BINARY_EXPRESSION(add, return t1 + t2);
		
		BINARY_EXPRESSION(sub, return t1 - t2);

		BINARY_EXPRESSION(mul, return t1 * t2);

		BINARY_EXPRESSION(div, return t1 / t2);

		BINARY_EXPRESSION(idiv, return int(t1 / t2));

		BINARY_EXPRESSION(mod, return t1 - t2 * int(t1/t2));

		BINARY_EXPRESSION(band, return int(t1) & int(t2));

		BINARY_EXPRESSION(bor, return int(t1) | int(t2));

		BINARY_EXPRESSION(bxor, return int(t1) ^ int(t2));

		BINARY_EXPRESSION(sl, return int(t1) << int(t2));

		BINARY_EXPRESSION(sr, return int(t1) >> int(t2));

		BINARY_EXPRESSION(concat, return std::make_shared<std::string>(*t1 + *t2));
		
		BINARY_EXPRESSION(add_assign,
			t1->value += t2;
			return t1;
		);
		
		BINARY_EXPRESSION(sub_assign,
			t1->value -= t2;
			return t1;
		);
		
		BINARY_EXPRESSION(mul_assign,
			t1->value *= t2;
			return t1;
		);
		
		BINARY_EXPRESSION(div_assign,
			t1->value /= t2;
			return t1;
		);
		
		BINARY_EXPRESSION(idiv_assign,
			t1->value = int(t1->value / t2);
			return t1;
		);
		
		BINARY_EXPRESSION(mod_assign,
			t1->value = t1->value - t2 * int(t1->value/t2);;
			return t1;
		);
		
		BINARY_EXPRESSION(band_assign,
			t1->value = int(t1->value) & int(t2);
			return t1;
		);
		
		BINARY_EXPRESSION(bor_assign,
			t1->value = int(t1->value) | int(t2);
			return t1;
		);
		
		BINARY_EXPRESSION(bxor_assign,
			t1->value = int(t1->value) ^ int(t2);
			return t1;
		);
		
		BINARY_EXPRESSION(bsl_assign,
			t1->value = int(t1->value) << int(t2);
			return t1;
		);
		
		BINARY_EXPRESSION(bsr_assign,
			t1->value = int(t1->value) >> int(t2);
			return t1;
		);

		BINARY_EXPRESSION(concat_assign,
			t1->value = std::make_shared<std::string>(*t1->value + *t2);
			return t1;
		);
		
		BINARY_EXPRESSION(assign, return assign(t1, t2));
		
		BINARY_EXPRESSION(eq, return !lt(t1, t2) && !lt(t2, t1));
		
		BINARY_EXPRESSION(ne, return lt(t1, t2) || lt(t2, t1));
		
		BINARY_EXPRESSION(lt, return lt(t1, t2));
		
		BINARY_EXPRESSION(gt, return lt(t2, t1));
		
		BINARY_EXPRESSION(le, return !lt(t2, t1));
		
		BINARY_EXPRESSION(ge, return !lt(t1, t2));
		
		TERNARY_EXPRESSION(ternary, return t1 ? t2 : t3);
		
		//template class generic_expression<ternary_op, string, number, string, string>;
		
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
		
		template<typename R>
		class assign_array_expression: public expression<array_variable_ptr> {
		private:
			std::string _type;
			array_variable_expression::ptr _expr1;
			array_variable_expression::ptr _expr2;
			
			template <typename T>
			static std::shared_ptr<variable_impl<T> > clone(const std::shared_ptr<variable_impl<T> >& t) {
				return std::make_shared<variable_impl<T> >(t->value);
			}
			
			template <typename T>
			static void shallow_copy_array(array& dst, const array_variable_ptr& src) {
				for (const variable_ptr& v : src->value) {
					dst.push_back(clone(v->static_downcast<T>()));
				}
			}
			
			static array_variable_ptr clone(std::string_view type, const array_variable_ptr& src) {
				array dst;
				
				switch (type.front()) {
					case '[':
						{
							std::string_view inner_type = type.substr(1, type.size() - 2);
							for (const variable_ptr& v : src->value) {
									dst.push_back(clone(inner_type, v->static_downcast<array>()));
							}
						}
						break;
					case '(':
						shallow_copy_array<function>(dst, src);
						break;
					default:
						if (type == "number") {
							shallow_copy_array<number>(dst, src);
						} else if (type == "string") {
							shallow_copy_array<string>(dst, src);
						}
						break;
				}
				
				return std::make_shared<array_variable>(std::move(dst));
			}
		public:
			assign_array_expression(std::string type, array_variable_expression::ptr expr1, array_variable_expression::ptr expr2) :
				_type(std::move(type)),
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(_expr1->evaluate(context)->value = clone(_type, _expr2->evaluate(context))->value);
			}
		};
		
		/*
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
	}
/*
	{"(", reserved_token::open_round},
	{")", reserved_token::close_round},
*/
}
