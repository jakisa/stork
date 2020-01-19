#include "expression.hpp"
#include <type_traits>

namespace stork {
	namespace {
		template<typename T>
		struct remove_cvref {
			using type = typename std::remove_reference<typename std::remove_cv<T>::type>::type;
		};
	
		template<class V, typename T>
		struct is_boxed {
			static const bool value = false;
		};
		
		template<typename T>
		struct is_boxed<std::shared_ptr<variable_impl<T> >, T> {
			static const bool value = true;
		};
	
		template<typename To, typename From>
		auto convert(From&& from) {
			if constexpr(std::is_void<To>::value) {
				return;
			} else if constexpr(std::is_same<To, typename remove_cvref<From>::type>::value) {
				return std::forward<From>(from);
			} else if constexpr(is_boxed<From, To>::value) {
				return (const To&)(from->value);
			} else {
				static_assert(std::is_same<To, string>::value);
				return to_string(from);
			}
		}
		
		/*
		//Removed while work is in progress, to disable unused function warnings.
		
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
	
		template<typename R, typename T>
		class global_variable_expression: public expression<R> {
		private:
			int _idx;
		public:
			global_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(context.global(_idx)->template static_pointer_downcast<T>());
			}
		};
		
		template<typename R, typename T>
		class local_variable_expression: public expression<R> {
		private:
			int _idx;
		public:
			local_variable_expression(int idx) :
				_idx(idx)
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(context.local(_idx)->template static_pointer_downcast<T>());
			}
		};
		
		template<typename R, typename T>
		class constant_expression: public expression<R> {
		private:
			R _c;
		public:
			constant_expression(T c) :
				_c(convert<R>(std::move(c)))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return _c;
			}
		};

		template<class O, typename R, typename... Ts>
		class generic_expression: public expression<R> {
		private:
			std::tuple<typename expression<Ts>::ptr...> _exprs;
			
			template<typename... Exprs>
			R evaluate_tuple(runtime_context& context, const Exprs&... exprs) const {
				return convert<R>(O()(std::move(exprs->evaluate(context))...));
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
		
		BINARY_EXPRESSION(comma, return (t1, t2));
		
		TERNARY_EXPRESSION(ternary, return t1 ? t2 : t3);
		
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
					dst.push_back(clone(v->static_pointer_downcast<T>()));
				}
			}
			
			static array_variable_ptr clone(std::string_view type, const array_variable_ptr& src) {
				array dst;
				
				switch (type.front()) {
					case '[':
						{
							std::string_view inner_type = type.substr(1, type.size() - 2);
							for (const variable_ptr& v : src->value) {
									dst.push_back(clone(inner_type, v->static_pointer_downcast<array_variable_ptr>()));
							}
						}
						break;
					case '(':
						shallow_copy_array<function_variable_ptr>(dst, src);
						break;
					default:
						if (type == "number") {
							shallow_copy_array<number_variable_ptr>(dst, src);
						} else if (type == "string") {
							shallow_copy_array<string_variable_ptr>(dst, src);
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
		
		template<typename R, typename T>
		class index_expression: public expression<R>{
		private:
			array_variable_expression::ptr _expr1;
			number_expression::ptr _expr2;
		public:
			index_expression(array_variable_expression::ptr expr1, number_expression::ptr expr2):
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
		
			R evaluate(runtime_context& context) const override {
				return convert<R>(
					_expr1->evaluate(context)->value[
						int(_expr2->evaluate(context))
					]->template static_pointer_downcast<T>()
				);
			}
		};
		
		template<typename R, typename T>
		class call_expression: public expression<R>{
		private:
			function_variable_expression::ptr _fexpr;
			std::vector<variable_expression::ptr> _exprs;
		public:
			call_expression(
				function_variable_expression::ptr fexpr,
				std::vector<variable_expression::ptr> exprs
			):
				_fexpr(std::move(fexpr)),
				_exprs(std::move(exprs))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				for (size_t i = _exprs.size(); i; --i) {
					context.push(_exprs[i-1]->evaluate(context));
				}

				function_variable_ptr f = _fexpr->evaluate(context);
				
				context.call();
				f->value(context);
				
				return convert<R>(
					context.end_function(
						_exprs.size()
					)->template static_pointer_downcast<T>()
				);
			}
		};
		
		template class call_expression<number, number_variable_ptr>;
	}
}
