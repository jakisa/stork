#include "expression.hpp"
#include <type_traits>
#include "expression_tree.hpp"
#include "expression_tree_parser.hpp"
#include "helpers.hpp"
#include "errors.hpp"

namespace stork {
	namespace {
		template<class V, typename T>
		struct is_boxed {
			static const bool value = false;
		};
		
		template<typename T>
		struct is_boxed<std::shared_ptr<variable_impl<T> >, T> {
			static const bool value = true;
		};
		
		string convert_to_string(number n) {
			std::string str;
			if (n == int(n)) {
				str = std::to_string(int(n));
			} else {
				str = std::to_string(n);
			}
			return std::make_shared<std::string>(std::move(str));
		}
		
		string convert_to_string(const lnumber& v) {
			return convert_to_string(v->value);
		}
	
		template<typename To, typename From>
		auto convert(From&& from) {
			if constexpr(std::is_convertible<From, To>::value) {
				return std::forward<From>(from);
			} else if constexpr(is_boxed<From, To>::value) {
				return (const To&)(from->value);
			} else if constexpr(std::is_same<To, string>::value) {
				return convert_to_string(from);
			} else {
				static_assert(std::is_void<To>::value);
			}
		}
		
		
		number lt(number n1, number n2) {
			return n1 < n2;
		}
		
		number lt(string s1, string s2) {
			return *s1 < *s2;
		}
	
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
		
		template<typename R>
		class constant_expression: public expression<R> {
		private:
			R _c;
		public:
			template <typename T>
			constant_expression(T c) :
				_c(convert<R>(std::move(c)))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return _c;
			}
		};
		
		template<>
		class constant_expression<void>: public expression<void> {
		public:
			template <typename T>
			constant_expression(T)
			{
			}
			
			void evaluate(runtime_context& context) const override {
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
		
		template<class O, typename... Ts>
		class generic_expression<O, void, Ts...>: public expression<void> {
		private:
			std::tuple<typename expression<Ts>::ptr...> _exprs;
			
			template<typename... Exprs>
			void evaluate_tuple(runtime_context& context, const Exprs&... exprs) const {
				O()(std::move(exprs->evaluate(context))...);
			}
		public:
			generic_expression(typename expression<Ts>::ptr... exprs) :
				_exprs(std::move(exprs)...)
			{
			}
			
			void evaluate(runtime_context& context) const override {
				std::apply(
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

		BINARY_EXPRESSION(bsl, return int(t1) << int(t2));

		BINARY_EXPRESSION(bsr, return int(t1) >> int(t2));

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
		
		BINARY_EXPRESSION(assign,
			t1->value = clone_variable_value(t2);
			return t1;
		);
		
		BINARY_EXPRESSION(eq, return !lt(t1, t2) && !lt(t2, t1));
		
		BINARY_EXPRESSION(ne, return lt(t1, t2) || lt(t2, t1));
		
		BINARY_EXPRESSION(lt, return lt(t1, t2));
		
		BINARY_EXPRESSION(gt, return lt(t2, t1));
		
		BINARY_EXPRESSION(le, return !lt(t2, t1));
		
		BINARY_EXPRESSION(ge, return !lt(t1, t2));
		
		TERNARY_EXPRESSION(ternary, return t1 ? t2 : t3);
		
		template<typename R, typename T1, typename T2>
		class comma_expression: public expression<R> {
		private:
			typename expression<T1>::ptr _expr1;
			typename expression<T2>::ptr _expr2;
		public:
			comma_expression(typename expression<T1>::ptr expr1, typename expression<T2>::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				_expr1->evaluate(context);
				return convert<R>(_expr2->evaluate(context));
			}
		};
		
		template<typename T1, typename T2>
		class comma_expression<void, T1, T2>: public expression<void> {
		private:
			typename expression<T1>::ptr _expr1;
			typename expression<T2>::ptr _expr2;
		public:
			comma_expression(typename expression<T1>::ptr expr1, typename expression<T2>::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			void evaluate(runtime_context& context) const override {
				_expr1->evaluate(context);
				_expr2->evaluate(context);
			}
		};
		
		template<typename R, typename T1, typename T2>
		class land_expression: public expression<R> {
		private:
			typename expression<T1>::ptr _expr1;
			typename expression<T2>::ptr _expr2;
		public:
			land_expression(typename expression<T1>::ptr expr1, typename expression<T2>::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(_expr1->evaluate(context) && _expr2->evaluate(context));
			}
		};
		
		template<typename R, typename T1, typename T2>
		class lor_expression: public expression<R> {
		private:
			typename expression<T1>::ptr _expr1;
			typename expression<T2>::ptr _expr2;
		public:
			lor_expression(typename expression<T1>::ptr expr1, typename expression<T2>::ptr expr2) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(_expr1->evaluate(context) || _expr2->evaluate(context));
			}
		};
		
		
		template<typename R, typename T>
		class index_expression: public expression<R>{
		private:
			expression<larray>::ptr _expr1;
			expression<number>::ptr _expr2;
		public:
			index_expression(expression<larray>::ptr expr1, expression<number>::ptr expr2):
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
		
			R evaluate(runtime_context& context) const override {
				larray arr = _expr1->evaluate(context);
				int idx = int(_expr2->evaluate(context));
				
				runtime_assertion(idx >= 0 && idx < arr->value.size(), "Subscript out of range");

				return convert<R>(
					arr->value[idx]->template static_pointer_downcast<T>()
				);
			}
		};
		
		template<typename R, typename T>
		class call_expression: public expression<R>{
		private:
			expression<lfunction>::ptr _fexpr;
			std::vector<expression<lvalue>::ptr> _exprs;
		public:
			call_expression(
				expression<lfunction>::ptr fexpr,
				std::vector<expression<lvalue>::ptr> exprs
			):
				_fexpr(std::move(fexpr)),
				_exprs(std::move(exprs))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				for (size_t i = _exprs.size(); i; --i) {
					context.push(_exprs[i-1]->evaluate(context));
				}

				lfunction f = _fexpr->evaluate(context);
				
				context.call();
				f->value(context);
				
				return convert<R>(
					std::static_pointer_cast<variable_impl<T> >(
						context.end_function(
							_exprs.size()
						)
					)
				);
			}
		};
		
		template<>
		class call_expression<void, void>: public expression<void> {
		private:
			expression<lfunction>::ptr _fexpr;
			std::vector<expression<lvalue>::ptr> _exprs;
		public:
			call_expression(
				expression<lfunction>::ptr fexpr,
				std::vector<expression<lvalue>::ptr> exprs
			):
				_fexpr(std::move(fexpr)),
				_exprs(std::move(exprs))
			{
			}
			
			void evaluate(runtime_context& context) const override {
				for (size_t i = _exprs.size(); i; --i) {
					context.push(_exprs[i-1]->evaluate(context));
				}

				lfunction f = _fexpr->evaluate(context);
				
				context.call();
				f->value(context);
				
				context.end_function(_exprs.size());
			}
		};
		
		template<typename R, typename T>
		class param_expression: public expression<R> {
		private:
			typename expression<T>::ptr _expr;
		public:
			param_expression(typename expression<T>::ptr expr) :
				_expr(std::move(expr))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(std::make_shared<variable_impl<T> >(_expr->evaluate(context)));
			}
		};
		
		template<typename R>
		class param_expression<R, lvalue>: public expression<R> {
		private:
			expression<lvalue>::ptr _expr;
		public:
			param_expression(expression<lvalue>::ptr expr) :
				_expr(std::move(expr))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(_expr->evaluate(context)->clone());
			}
		};
		
		class expression_builder_error {
		};
		
		template<typename R>
		struct expression_builder;
		
		template<>
		struct expression_builder<void> {
			static expression<void>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template<>
		struct expression_builder<number> {
			static expression<number>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template<>
		struct expression_builder<string> {
			static expression<string>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template<>
		struct expression_builder<lnumber> {
			static expression<lnumber>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template<>
		struct expression_builder<lstring> {
			static expression<lstring>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template<>
		struct expression_builder<lfunction> {
			static expression<lfunction>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template<>
		struct expression_builder<larray> {
			static expression<larray>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template<>
		struct expression_builder<lvalue> {
			static expression<lvalue>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};

#define CHECK_UNARY_OPERATION(name, T1)\
							case node_operation::name:\
								return expression_ptr(\
									std::make_unique<name##_expression<R, T1> > (\
										expression_builder<T1>::build_expression(np->get_children()[0], context)\
									)\
								);

#define CHECK_BINARY_OPERATION(name, T1, T2)\
							case node_operation::name:\
								return expression_ptr(\
									std::make_unique<name##_expression<R, T1, T2> > (\
										expression_builder<T1>::build_expression(np->get_children()[0], context),\
										expression_builder<T2>::build_expression(np->get_children()[1], context)\
									)\
								);

#define CHECK_TERNARY_OPERATION(name, T1, T2, T3)\
							case node_operation::name:\
								return expression_ptr(\
									std::make_unique<name##_expression<R, T1, T2, T3> > (\
										expression_builder<T1>::build_expression(np->get_children()[0], context),\
										expression_builder<T2>::build_expression(np->get_children()[1], context),\
										expression_builder<T3>::build_expression(np->get_children()[2], context)\
									)\
								);

#define CHECK_COMPARISON_OPERATION(name)\
							case node_operation::name:\
								if (\
									np->get_children()[0]->get_type_id() == type_registry::get_number_handle() &&\
									np->get_children()[1]->get_type_id() == type_registry::get_number_handle()\
								) {\
									return expression_ptr(\
										std::make_unique<name##_expression<R, number, number> > (\
											expression_builder<number>::build_expression(np->get_children()[0], context),\
											expression_builder<number>::build_expression(np->get_children()[1], context)\
										)\
									);\
								} else {\
									return expression_ptr(\
										std::make_unique<name##_expression<R, string, string> > (\
											expression_builder<string>::build_expression(np->get_children()[0], context),\
											expression_builder<string>::build_expression(np->get_children()[1], context)\
										)\
									);\
								}

#define CHECK_INDEX_OPERATION(T)\
							case node_operation::index:\
								return expression_ptr(\
									std::make_unique<index_expression<R, T> >(\
										expression_builder<larray>::build_expression(np->get_children()[0], context),\
										expression_builder<number>::build_expression(np->get_children()[1], context)\
									)\
								);

#define CHECK_CALL_OPERATION(T)\
							case node_operation::call:\
							{\
								std::vector<expression<lvalue>::ptr> arguments;\
								for (size_t i = 1; i < np->get_children().size(); ++i) {\
									arguments.push_back(\
										expression_builder<lvalue>::build_expression(np->get_children()[i], context)\
									);\
								}\
								return expression_ptr(\
									std::make_unique<call_expression<R, T> >(\
										expression_builder<lfunction>::build_expression(np->get_children()[0], context),\
										std::move(arguments)\
									)\
								);\
							}

		template <typename R>
		struct number_expression_builder {
			using expression_ptr = typename expression<R>::ptr;

			static expression_ptr build_expression(const node_ptr& np, compiler_context& context) {
				if (np->get_type_id() != type_registry::get_number_handle() || np->is_lvalue()) {
					return nullptr;
				}
			
				return std::visit(overloaded{
					[&](double d) {
						return expression_ptr(
							std::make_unique<constant_expression<R> > (
								d
							)
						);
					},
					[&](node_operation op) {
						switch (op) {
							CHECK_UNARY_OPERATION(postinc, lnumber);
							CHECK_UNARY_OPERATION(postdec, lnumber);
							CHECK_UNARY_OPERATION(positive, number);
							CHECK_UNARY_OPERATION(negative, number);
							CHECK_UNARY_OPERATION(bnot, number);
							CHECK_UNARY_OPERATION(lnot, number);
							CHECK_BINARY_OPERATION(add, number, number);
							CHECK_BINARY_OPERATION(sub, number, number);
							CHECK_BINARY_OPERATION(mul, number, number);
							CHECK_BINARY_OPERATION(div, number, number);
							CHECK_BINARY_OPERATION(idiv, number, number);
							CHECK_BINARY_OPERATION(mod, number, number);
							CHECK_BINARY_OPERATION(band, number, number);
							CHECK_BINARY_OPERATION(bor, number, number);
							CHECK_BINARY_OPERATION(bxor, number, number);
							CHECK_BINARY_OPERATION(bsl, number, number);
							CHECK_BINARY_OPERATION(bsr, number, number);
							CHECK_BINARY_OPERATION(comma, void, number);
							CHECK_BINARY_OPERATION(land, number, number);
							CHECK_BINARY_OPERATION(lor, number, number);
							CHECK_COMPARISON_OPERATION(eq);
							CHECK_COMPARISON_OPERATION(ne);
							CHECK_COMPARISON_OPERATION(lt);
							CHECK_COMPARISON_OPERATION(gt);
							CHECK_COMPARISON_OPERATION(le);
							CHECK_COMPARISON_OPERATION(ge);
							CHECK_TERNARY_OPERATION(ternary, number, number, number);
							CHECK_INDEX_OPERATION(lnumber);
							CHECK_CALL_OPERATION(number);
							default:
								return expression_ptr();
						}
					},
					[](const auto&) {
						return expression_ptr();
					}
				}, np->get_value());
			}
		};
		
		template <typename R>
		struct string_expression_builder {
			using expression_ptr = typename expression<R>::ptr;
			
			static expression_ptr build_expression(const node_ptr& np, compiler_context& context) {
				if (np->get_type_id() != type_registry::get_string_handle() || np->is_lvalue()) {
					return nullptr;
				}
				
				return std::visit(overloaded{
					[&](const std::string& str) {
						return expression_ptr(
							std::make_unique<constant_expression<R> > (
								std::make_shared<std::string>(str)
							)
						);
					},
					[&](node_operation op) {
						switch (op) {
							CHECK_BINARY_OPERATION(concat, string, string);
							CHECK_BINARY_OPERATION(comma, void, string);
							CHECK_TERNARY_OPERATION(ternary, number, string, string);
							CHECK_INDEX_OPERATION(lstring);
							CHECK_CALL_OPERATION(string);
							default:
								return expression_ptr();
						}
					},
					[](const auto&) {
						return expression_ptr();
					}
				}, np->get_value());
			}
		};

		template <typename R>
		struct lnumber_expression_builder {
			using expression_ptr = typename expression<R>::ptr;

			static expression_ptr build_expression(const node_ptr& np, compiler_context& context) {
				if (np->get_type_id() != type_registry::get_number_handle() || !np->is_lvalue()) {
					return nullptr;
				}
			
				return std::visit(overloaded{
					[&](const identifier& id) {
						const identifier_info* id_info = context.find(id.name);
						if (id_info->is_global()) {
							return expression_ptr(
								std::make_unique<global_variable_expression<R, lnumber> >(
									id_info->index()
								)
							);
						} else {
							return expression_ptr(
								std::make_unique<local_variable_expression<R, lnumber> >(
									id_info->index()
								)
							);
						}
					},
					[&](node_operation op) {
						switch (op) {
							CHECK_UNARY_OPERATION(preinc, lnumber);
							CHECK_UNARY_OPERATION(predec, lnumber);
							CHECK_BINARY_OPERATION(assign, lnumber, number);
							CHECK_BINARY_OPERATION(add_assign, lnumber, number);
							CHECK_BINARY_OPERATION(sub_assign, lnumber, number);
							CHECK_BINARY_OPERATION(mul_assign, lnumber, number);
							CHECK_BINARY_OPERATION(div_assign, lnumber, number);
							CHECK_BINARY_OPERATION(idiv_assign, lnumber, number);
							CHECK_BINARY_OPERATION(mod_assign, lnumber, number);
							CHECK_BINARY_OPERATION(band_assign, lnumber, number);
							CHECK_BINARY_OPERATION(bor_assign, lnumber, number);
							CHECK_BINARY_OPERATION(bxor_assign, lnumber, number);
							CHECK_BINARY_OPERATION(bsl_assign, lnumber, number);
							CHECK_BINARY_OPERATION(bsr_assign, lnumber, number);
							CHECK_BINARY_OPERATION(comma, void, lnumber);
							CHECK_TERNARY_OPERATION(ternary, number, lnumber, lnumber);
							CHECK_INDEX_OPERATION(lnumber);
							default:
								return expression_ptr();
						}
					},
					[](const auto&) {
						return expression_ptr();
					}
				}, np->get_value());
			}
		};
		
		template <typename R>
		struct lstring_expression_builder {
			using expression_ptr = typename expression<R>::ptr;

			static expression_ptr build_expression(const node_ptr& np, compiler_context& context) {
				if (np->get_type_id() != type_registry::get_string_handle() || !np->is_lvalue()) {
					return nullptr;
				}
			
				return std::visit(overloaded{
					[&](const identifier& id) {
						const identifier_info* id_info = context.find(id.name);
						if (id_info->is_global()) {
							return expression_ptr(
								std::make_unique<global_variable_expression<R, lstring> >(
									id_info->index()
								)
							);
						} else {
							return expression_ptr(
								std::make_unique<local_variable_expression<R, lstring> >(
									id_info->index()
								)
							);
						}
					},
					[&](node_operation op) {
						switch (op) {
							CHECK_BINARY_OPERATION(assign, lstring, string);
							CHECK_BINARY_OPERATION(concat_assign, lstring, string);
							CHECK_BINARY_OPERATION(comma, void, lstring);
							CHECK_TERNARY_OPERATION(ternary, number, lstring, lstring);
							CHECK_INDEX_OPERATION(lstring);
							default:
								return expression_ptr();
						}
					},
					[](const auto&) {
						return expression_ptr();
					}
				}, np->get_value());
			}
		};
		
		template <typename R>
		struct lfunction_expression_builder {
			using expression_ptr = typename expression<R>::ptr;

			static expression_ptr build_expression(const node_ptr& np, compiler_context& context) {
				if (!std::holds_alternative<function_type>(*np->get_type_id())) {
					return nullptr;
				}
			
				return std::visit(overloaded{
					[&](const identifier& id) {
						const identifier_info* id_info = context.find(id.name);
						if (id_info->is_global()) {
							return expression_ptr(
								std::make_unique<global_variable_expression<R, lfunction> >(
									id_info->index()
								)
							);
						} else {
							return expression_ptr(
								std::make_unique<local_variable_expression<R, lfunction> >(
									id_info->index()
								)
							);
						}
					},
					[&](node_operation op) {
						switch (op) {
							CHECK_BINARY_OPERATION(assign, lfunction, lfunction);
							CHECK_BINARY_OPERATION(comma, void, lfunction);
							CHECK_TERNARY_OPERATION(ternary, number, lfunction, lfunction);
							CHECK_INDEX_OPERATION(lfunction);
							default:
								return expression_ptr();
						}
					},
					[](const auto&) {
						return expression_ptr();
					}
				}, np->get_value());
			}
		};

		template <typename R>
		struct larray_expression_builder {
			using expression_ptr = typename expression<R>::ptr;

			static expression_ptr build_expression(const node_ptr& np, compiler_context& context) {
				if (!std::holds_alternative<array_type>(*np->get_type_id())) {
					return nullptr;
				}
			
				return std::visit(overloaded{
					[&](const identifier& id) {
						const identifier_info* id_info = context.find(id.name);
						if (id_info->is_global()) {
							return expression_ptr(
								std::make_unique<global_variable_expression<R, larray> >(
									id_info->index()
								)
							);
						} else {
							return expression_ptr(
								std::make_unique<local_variable_expression<R, larray> >(
									id_info->index()
								)
							);
						}
					},
					[&](node_operation op) {
						switch (op) {
							CHECK_BINARY_OPERATION(assign, larray, larray);
							CHECK_BINARY_OPERATION(comma, void, larray);
							CHECK_TERNARY_OPERATION(ternary, number, larray, larray);
							CHECK_INDEX_OPERATION(larray);
							default:
								return expression_ptr();
						}
					},
					[](const auto&) {
						return expression_ptr();
					}
				}, np->get_value());
			}
		};
		
		template <typename R>
		struct lvalue_expression_builder {
			using expression_ptr = typename expression<R>::ptr;

			static expression_ptr build_expression(const node_ptr& np, compiler_context& context) {
				return std::visit(overloaded{
					[&](node_operation op) {
						switch (op) {
							case node_operation::param:
							{
								const node_ptr& child = np->get_children()[0];
								if (child->get_type_id() == type_registry::get_number_handle()) {
									return expression_ptr(
										std::make_unique<param_expression<R, number> >(
											expression_builder<number>::build_expression(child, context)
										)
									);
								} else if (child->get_type_id() == type_registry::get_string_handle()) {
									return expression_ptr(
										std::make_unique<param_expression<R, string> >(
											expression_builder<string>::build_expression(child, context)
										)
									);
								} else {
									return expression_ptr(
										std::make_unique<param_expression<R, lvalue> >(
											expression_builder<lvalue>::build_expression(child, context)
										)
									);
								}
								break;
							}
							default:
								return expression_ptr();
						}
					},
					[](const auto&) {
						return expression_ptr();
					}
				}, np->get_value());
			}
		};

#define CHECK_RETURN_EXPRESSION(R, T)\
			if (expression<R>::ptr ret = T##_expression_builder<R>::build_expression(np, context)) {\
				return ret;\
			}
		
		expression<void>::ptr expression_builder<void>::build_expression(const node_ptr& np, compiler_context& context) {
			CHECK_RETURN_EXPRESSION(void, lnumber);
			CHECK_RETURN_EXPRESSION(void, lstring);
			CHECK_RETURN_EXPRESSION(void, lfunction);
			CHECK_RETURN_EXPRESSION(void, larray);
			CHECK_RETURN_EXPRESSION(void, lvalue);
			CHECK_RETURN_EXPRESSION(void, number);
			CHECK_RETURN_EXPRESSION(void, string);
			throw expression_builder_error();
		};
		
		expression<number>::ptr expression_builder<number>::build_expression(const node_ptr& np, compiler_context& context) {
			CHECK_RETURN_EXPRESSION(number, lnumber);
			CHECK_RETURN_EXPRESSION(number, number);
			throw expression_builder_error();
		};
		
		expression<string>::ptr expression_builder<string>::build_expression(const node_ptr& np, compiler_context& context) {
			CHECK_RETURN_EXPRESSION(string, lnumber);
			CHECK_RETURN_EXPRESSION(string, lstring);
			CHECK_RETURN_EXPRESSION(string, number);
			CHECK_RETURN_EXPRESSION(string, string);
			throw expression_builder_error();
		};
		
		expression<lnumber>::ptr expression_builder<lnumber>::build_expression(
			const node_ptr& np, compiler_context& context
		) {
			CHECK_RETURN_EXPRESSION(lnumber, lnumber);
			throw expression_builder_error();
		};
		
		expression<lstring>::ptr expression_builder<lstring>::build_expression(const node_ptr& np, compiler_context& context) {
			CHECK_RETURN_EXPRESSION(lstring, lstring);
			throw expression_builder_error();
		};
		
		expression<lfunction>::ptr expression_builder<lfunction>::build_expression(
			const node_ptr& np,
			compiler_context& context
		) {
			CHECK_RETURN_EXPRESSION(lfunction, lfunction);
			throw expression_builder_error();
		};
		
		expression<larray>::ptr expression_builder<larray>::build_expression(
			const node_ptr& np,
			compiler_context& context
		) {
			CHECK_RETURN_EXPRESSION(larray, larray);
			throw expression_builder_error();
		};
		
		expression<lvalue>::ptr expression_builder<lvalue>::build_expression(
			const node_ptr& np,
			compiler_context& context
		) {
			CHECK_RETURN_EXPRESSION(lvalue, lnumber);
			CHECK_RETURN_EXPRESSION(lvalue, lstring);
			CHECK_RETURN_EXPRESSION(lvalue, lfunction);
			CHECK_RETURN_EXPRESSION(lvalue, larray);
			CHECK_RETURN_EXPRESSION(lvalue, lvalue);
			throw expression_builder_error();
		};
	
		template <typename R>
		typename expression<R>::ptr build_expression(type_handle type_id, compiler_context& context, tokens_iterator& it) {
			size_t line_number = it->get_line_number();
			size_t char_index = it->get_char_index();
			
			try {
				return expression_builder<R>::build_expression(
					parse_expression_tree(context, it, type_id, false, true, false), context
				);
			} catch (const expression_builder_error&) {
				throw compiler_error("Expression building failed", line_number, char_index);
			}
		}
	}

	expression<void>::ptr build_void_expression(compiler_context& context, tokens_iterator& it) {
		return build_expression<void>(type_registry::get_void_handle(), context, it);
	}
	
	expression<number>::ptr build_number_expression(compiler_context& context, tokens_iterator& it) {
		return build_expression<number>(type_registry::get_number_handle(), context, it);
	}
}
