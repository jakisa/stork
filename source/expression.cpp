#include "expression.hpp"
#include <type_traits>
#include "expression_tree.hpp"
#include "expression_tree_parser.hpp"
#include "helpers.hpp"

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
		
		/*
		string to_string(number n) {
			return std::make_shared<std::string>(std::to_string(n));
		}
		
		string to_string(number_variable_cptr v) {
			return to_string(v->value);
		}
		*/
	
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
		
		template<typename R>
		class assign_array_expression: public expression<array_variable_ptr> {
		private:
			type_handle _type_id;
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
			
			static array_variable_ptr clone(type_handle type_id, const array_variable_ptr& src) {
				array dst;
				
				std::visit(overloaded{
					[&](const function_type&) {
						shallow_copy_array<function_variable_ptr>(dst, src);
					},
					[&](const array_type& at) {
						for (const variable_ptr& v : src->value) {
							dst.push_back(clone(at.inner_type_id, v->static_pointer_downcast<array_variable_ptr>()));
						}
					},
					[&](simple_type st) {
						switch (st) {
							case simple_type::number:
								shallow_copy_array<number_variable_ptr>(dst, src);
								break;
							case simple_type::string:
								shallow_copy_array<number_variable_ptr>(dst, src);
								break;
							default:
								break;
						}
					}
				}, *type_id);
				
				return std::make_shared<array_variable>(std::move(dst));
			}
		public:
			assign_array_expression(type_handle type_id, array_variable_expression::ptr expr1, array_variable_expression::ptr expr2) :
				_type_id(type_id),
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(_expr1->evaluate(context)->value = clone(_type_id, _expr2->evaluate(context))->value);
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
		
		template<>
		class call_expression<void, void>: public expression<void>{
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
			
			void evaluate(runtime_context& context) const override {
				for (size_t i = _exprs.size(); i; --i) {
					context.push(_exprs[i-1]->evaluate(context));
				}

				function_variable_ptr f = _fexpr->evaluate(context);
				
				context.call();
				f->value(context);
				
				context.end_function(_exprs.size());
			}
		};
		
		template <typename R>
		struct expression_builder;
		
		template <>
		struct expression_builder<void> {
			static expression<void>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template <>
		struct expression_builder<number> {
			static expression<number>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template <>
		struct expression_builder<number_variable_ptr> {
			static expression<number_variable_ptr>::ptr build_expression(const node_ptr& np, compiler_context& context);
		};
		
		template <>
		struct expression_builder<array_variable_ptr> {
			static expression<array_variable_ptr>::ptr build_expression(const node_ptr& np, compiler_context& context);
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


		template <typename R>
		struct lnumber_expression_builder {
			using expression_ptr = typename expression<R>::ptr;

			static expression_ptr build_expression(const node_ptr& np, compiler_context& context) {
				if (np->get_type_id() != type_registry::get_number_handle() || !np->is_lvalue()) {
					return expression_ptr();
				}
			
				return std::visit(overloaded{
					[&](const identifier& id) {
						const identifier_info* id_info = context.find(id.name);
						if (id_info->is_global()) {
							return expression_ptr(
								std::make_unique<global_variable_expression<R, number_variable_ptr> >(
									id_info->index()
								)
							);
						} else {
							return expression_ptr(
								std::make_unique<local_variable_expression<R, number_variable_ptr> >(
									id_info->index()
								)
							);
						}
					},
					[&](node_operation op) {
						switch (op) {
							CHECK_UNARY_OPERATION(preinc, number_variable_ptr);
							CHECK_UNARY_OPERATION(predec, number_variable_ptr);
							CHECK_BINARY_OPERATION(assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(add_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(sub_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(mul_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(div_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(idiv_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(mod_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(band_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(bor_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(bxor_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(bsl_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(bsr_assign, number_variable_ptr, number);
							CHECK_BINARY_OPERATION(comma, void, number_variable_ptr);
							case node_operation::index:
								return expression_ptr(
									std::make_unique<index_expression<R, number_variable_ptr> >(
										expression_builder<array_variable_ptr>::build_expression(np->get_children()[0], context),
										expression_builder<number>::build_expression(np->get_children()[1], context)
									)
								);
							CHECK_TERNARY_OPERATION(ternary, number, number_variable_ptr, number_variable_ptr);
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
		
		//TODO:
		expression<void>::ptr expression_builder<void>::build_expression(const node_ptr& np, compiler_context& context) {
			return nullptr;
		};
		
		//TODO:
		expression<number>::ptr expression_builder<number>::build_expression(const node_ptr& np, compiler_context& context) {
			return nullptr;
		};
		
		expression<number_variable_ptr>::ptr expression_builder<number_variable_ptr>::build_expression(
			const node_ptr& np, compiler_context& context
		) {
			return lnumber_expression_builder<number_variable_ptr>::build_expression(np, context);
		};
		
		//TODO:
		expression<array_variable_ptr>::ptr expression_builder<array_variable_ptr>::build_expression(
			const node_ptr& np,
			compiler_context& context
		) {
			return nullptr;
		};
	
		template <typename R>
		typename expression<R>::ptr build_expression(type_handle type_id, compiler_context& context, tokens_iterator& it) {
			//return to_expression<R>(parse_expression_tree(context, it, type_id, false, true, false), context);
			return nullptr;
		}
	}

	void_expression::ptr build_void_expression(compiler_context& context, tokens_iterator& it) {
		return build_expression<void>(type_registry::get_void_handle(), context, it);
	}
	
	number_expression::ptr build_number_expression(compiler_context& context, tokens_iterator& it) {
		return build_expression<number>(type_registry::get_number_handle(), context, it);
	}

}
