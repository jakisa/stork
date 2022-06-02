#include "expression.hpp"
#include <type_traits>
#include "expression_tree.hpp"
#include "expression_tree_parser.hpp"
#include "errors.hpp"
#include "runtime_context.hpp"
#include "tokenizer.hpp"
#include "compiler_context.hpp"
#include <cassert>

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
		
		template<typename T>
		struct remove_cvref {
			using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
		};
		
		template <typename T>
		auto unbox(T&& t) {
			if constexpr (std::is_same<typename remove_cvref<T>::type, larray>::value) {
				return clone_variable_value(t->value);
			} else {
				return t->value;
			}
		}
	
		template<typename To, typename From>
		auto convert(From&& from) {
			if constexpr(std::is_convertible<From, To>::value) {
				return std::forward<From>(from);
			} else if constexpr(is_boxed<From, To>::value) {
				return unbox(std::forward<From>(from));
			} else if constexpr(std::is_same<To, string>::value) {
				return convert_to_string(from);
			} else {
				static_assert(std::is_void<To>::value);
			}
		}
		
		template<typename From, typename To>
		struct is_convertible {
			static const bool value =
				std::is_convertible<From, To>::value ||
				is_boxed<From, To>::value ||
				(
					std::is_same<To, string>::value &&
					(
						std::is_same<From, number>::value ||
						std::is_same<From, lnumber>::value
					)
				) ||
				std::is_void<To>::value;
		};
		
		
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
		class function_expression: public expression<R> {
		private:
			int _idx;
		public:
			function_expression(int idx) :
				_idx(idx)
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(context.get_function(_idx));
			}
		};
		
		template<typename R, typename T>
		class constant_expression: public expression<R> {
		private:
			T _c;
		public:
			constant_expression(T c) :
				_c(std::move(c))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				return convert<R>(_c);
			}
		};

		template<class O, typename R, typename... Ts>
		class generic_expression: public expression<R> {
		private:
			std::tuple<typename expression<Ts>::ptr...> _exprs;
			
			template<typename... Exprs>
			R evaluate_tuple(runtime_context& context, const Exprs&... exprs) const {
				if constexpr(std::is_same<R, void>::value) {
					O()(std::move(exprs->evaluate(context))...);
				} else {
					return convert<R>(O()(std::move(exprs->evaluate(context))...));
				}
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
		
		UNARY_EXPRESSION(size,
			return t1->value.size();
		);
		
		UNARY_EXPRESSION(tostring,
			return convert_to_string(t1);
		);

#undef UNARY_EXPRESSION

#define BINARY_EXPRESSION(name, code)\
		struct name##_op {\
			template <typename T1, typename T2>\
			auto operator()(T1 t1, T2 t2) {\
				code;\
			}\
		};\
		template<typename R, typename T1, typename T2>\
		using name##_expression = generic_expression<name##_op, R, T1, T2>;

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
			t1->value = std::move(t2);
			return t1;
		);
		
		BINARY_EXPRESSION(eq, return !lt(t1, t2) && !lt(t2, t1));
		
		BINARY_EXPRESSION(ne, return lt(t1, t2) || lt(t2, t1));
		
		BINARY_EXPRESSION(lt, return lt(t1, t2));
		
		BINARY_EXPRESSION(gt, return lt(t2, t1));
		
		BINARY_EXPRESSION(le, return !lt(t2, t1));
		
		BINARY_EXPRESSION(ge, return !lt(t1, t2));

#undef BINARY_EXPRESSION

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
				
				if constexpr(std::is_same<R, void>::value) {
					_expr2->evaluate(context);
				} else {
					return convert<R>(_expr2->evaluate(context));
				}
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
		
		template<typename R, typename T1, typename T2, typename T3>
		class ternary_expression: public expression<R> {
		private:
			typename expression<T1>::ptr _expr1;
			typename expression<T2>::ptr _expr2;
			typename expression<T3>::ptr _expr3;
		public:
			ternary_expression(
				typename expression<T1>::ptr expr1,
				typename expression<T2>::ptr expr2,
				typename expression<T2>::ptr expr3
			) :
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2)),
				_expr3(std::move(expr3))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				if constexpr (std::is_same<R, void>::value) {
					_expr1->evaluate(context) ? _expr2->evaluate(context) : _expr3->evaluate(context);
				} else {
					return convert<R>(
						_expr1->evaluate(context) ?
						_expr2->evaluate(context) :
						_expr3->evaluate(context)
					);
				}
			}
		};
		
		template<typename R, typename A, typename T>
		class index_expression: public expression<R>{
		private:
			typename expression<A>::ptr _expr1;
			expression<number>::ptr _expr2;
			expression<lvalue>::ptr _init;
			
			static array& value(A& arr){
				if constexpr(std::is_same<larray, A>::value) {
					return arr->value;
				} else {
					static_assert(std::is_same<array, A>::value);
					return arr;
				}
			}
			
			static auto to_lvalue_impl(lvalue v) {
				if constexpr(std::is_same<larray, A>::value) {
					return v->static_pointer_downcast<T>();
				} else {
					static_assert(std::is_same<array, A>::value);
					return std::static_pointer_cast<variable_impl<T> >(v);
				}
			}
		public:
			index_expression(typename expression<A>::ptr expr1, expression<number>::ptr expr2, expression<lvalue>::ptr init):
				_expr1(std::move(expr1)),
				_expr2(std::move(expr2)),
				_init(std::move(init))
			{
			}
		
			R evaluate(runtime_context& context) const override {
				A arr = _expr1->evaluate(context);
				int idx = int(_expr2->evaluate(context));
				
				runtime_assertion(idx >= 0, "Negative index is invalid");
				
				while (idx >= value(arr).size()) {
					value(arr).push_back(_init->evaluate(context));
				}

				return convert<R>(
					to_lvalue_impl(value(arr)[idx])
				);
			}
		};
		
		
		template<typename R, typename A, typename T>
		class member_expression: public expression<R>{
		private:
			typename expression<A>::ptr _expr;
			size_t _idx;
			
			static tuple& value(A& arr){
				if constexpr(std::is_same<ltuple, A>::value) {
					return arr->value;
				} else {
					static_assert(std::is_same<tuple, A>::value);
					return arr;
				}
			}
			
			static auto to_lvalue_impl(lvalue v) {
				if constexpr(std::is_same<larray, A>::value) {
					return v->static_pointer_downcast<T>();
				} else {
					static_assert(std::is_same<array, A>::value);
					return std::static_pointer_cast<variable_impl<T> >(v);
				}
			}
		public:
			member_expression(typename expression<A>::ptr expr, size_t idx):
				_expr(std::move(expr)),
				_idx(idx)
			{
			}
		
			R evaluate(runtime_context& context) const override {
				A tup = _expr->evaluate(context);
				
				return convert<R>(
					to_lvalue_impl(value(tup)[_idx])
				);
			}
			
		};
		
		
		template<typename R, typename T>
		class call_expression: public expression<R>{
		private:
			expression<function>::ptr _fexpr;
			std::vector<expression<lvalue>::ptr> _exprs;
		public:
			call_expression(
				expression<function>::ptr fexpr,
				std::vector<expression<lvalue>::ptr> exprs
			):
				_fexpr(std::move(fexpr)),
				_exprs(std::move(exprs))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				std::vector<variable_ptr> params;
				params.reserve(_exprs.size());
			
				for (size_t i = 0; i < _exprs.size(); ++i) {
					params.push_back(_exprs[i]->evaluate(context));
				}
				
				function f = _fexpr->evaluate(context);
				
				if constexpr (std::is_same<R, void>::value) {
					context.call(f, std::move(params));
				} else {
					return convert<R>(std::move(
						std::static_pointer_cast<variable_impl<T> >(context.call(f, std::move(params)))->value
					));
				}
			}
		};
		
		template<typename R>
		class init_expression: public expression<R>{
		private:
			std::vector<expression<lvalue>::ptr> _exprs;
		public:
			init_expression(
				std::vector<expression<lvalue>::ptr> exprs
			):
				_exprs(std::move(exprs))
			{
			}
			
			R evaluate(runtime_context& context) const override {
				if constexpr(std::is_same<void, R>()) {
					for (const expression<lvalue>::ptr& expr : _exprs) {
						expr->evaluate(context);
					}
				} else if constexpr(std::is_same<array, R>() || std::is_same<tuple, R>()) {
					initializer_list lst;
					for (const expression<lvalue>::ptr& expr : _exprs) {
						lst.push_back(expr->evaluate(context));
					}
					return lst;
				}
			}
		};
	
		template <typename T>
		class param_expression: public expression<lvalue> {
		private:
			typename expression<T>::ptr _expr;
		public:
			param_expression(typename expression<T>::ptr expr) :
				_expr(std::move(expr))
			{
			}
			
			lvalue evaluate(runtime_context& context) const override {
				return std::make_shared<variable_impl<T> >(_expr->evaluate(context));
			}
		};
		
		struct expression_builder_error {
			expression_builder_error(){
			}
		};
		
		class tuple_initialization_expression: public expression<lvalue> {
		private:
			std::vector<expression<lvalue>::ptr> _exprs;
		public:
			tuple_initialization_expression(std::vector<expression<lvalue>::ptr> exprs) :
				_exprs(std::move(exprs))
			{
			}
			
			lvalue evaluate(runtime_context& context) const override {
				tuple ret;
				
				for (const expression<lvalue>::ptr& expr : _exprs) {
					ret.push_back(expr->evaluate(context));
				}
				
				return std::make_unique<variable_impl<tuple> >(std::move(ret));
			}
		};
		
		expression<lvalue>::ptr build_lvalue_expression(type_handle type_id, const node_ptr& np, compiler_context& context);
		
#define RETURN_EXPRESSION_OF_TYPE(T)\
	if constexpr(is_convertible<T, R>::value) {\
		return build_##T##_expression(np, context);\
	} else {\
		throw expression_builder_error();\
		return expression_ptr();\
	}

#define CHECK_IDENTIFIER(T1)\
	if (std::holds_alternative<identifier>(np->get_value())) {\
		const identifier& id = std::get<identifier>(np->get_value());\
		const identifier_info* info = context.find(id.name);\
		switch (info->get_scope()) {\
			case identifier_scope::global_variable:\
				return std::make_unique<global_variable_expression<R, T1> >(info->index());\
			case identifier_scope::local_variable:\
				return std::make_unique<local_variable_expression<R, T1> >(info->index());\
			case identifier_scope::function:\
				break;\
		}\
	}

#define CHECK_FUNCTION()\
	if (std::holds_alternative<identifier>(np->get_value())) {\
		const identifier& id = std::get<identifier>(np->get_value());\
		const identifier_info* info = context.find(id.name);\
		switch (info->get_scope()) {\
			case identifier_scope::global_variable:\
			case identifier_scope::local_variable:\
				break;\
			case identifier_scope::function:\
				return std::make_unique<function_expression<R> >(info->index());\
		}\
	}

#define CHECK_UNARY_OPERATION(name, T1)\
	case node_operation::name:\
		return expression_ptr(\
			std::make_unique<name##_expression<R, T1> > (\
				expression_builder<T1>::build_expression(np->get_children()[0], context)\
			)\
		);

#define CHECK_SIZE_OPERATION()\
	case node_operation::size:\
		if (std::holds_alternative<array_type>(*(np->get_children()[0]->get_type_id()))) {\
			return expression_ptr(\
				std::make_unique<size_expression<R, larray> > (\
					expression_builder<larray>::build_expression(np->get_children()[0], context)\
				)\
			);\
		} else {\
			return expression_ptr(\
				std::make_unique<constant_expression<R, number> >(1)\
			);\
		}

#define CHECK_TO_STRING_OPERATION()\
	case node_operation::tostring:\
		if (np->get_children()[0]->is_lvalue()) {\
			return expression_ptr(std::make_unique<tostring_expression<R, lvalue> > (\
				expression_builder<lvalue>::build_expression(np->get_children()[0], context)\
			));\
		}\
		return std::visit([&](const auto& t){\
			if constexpr(std::is_same_v<decltype(t), const simple_type&>) {\
				switch (t) {\
					case simple_type::number:\
						return expression_ptr(std::make_unique<tostring_expression<R, number> > (\
							expression_builder<number>::build_expression(np->get_children()[0], context)\
						));\
					case simple_type::string:\
						return expression_ptr(std::make_unique<tostring_expression<R, string> > (\
							expression_builder<string>::build_expression(np->get_children()[0], context)\
						));\
					case simple_type::nothing:\
						throw expression_builder_error();\
				}\
				return expression_ptr();\
			} else if constexpr(std::is_same_v<decltype(t), const function_type&>) {\
				return expression_ptr(std::make_unique<tostring_expression<R, function> > (\
					expression_builder<function>::build_expression(np->get_children()[0], context)\
				));\
			} else if constexpr(std::is_same_v<decltype(t), const array_type&>)  {\
				return expression_ptr(std::make_unique<tostring_expression<R, array> > (\
					expression_builder<array>::build_expression(np->get_children()[0], context)\
				));\
			} else if constexpr(std::is_same_v<decltype(t), const tuple_type&>) {\
				return expression_ptr(std::make_unique<tostring_expression<R, tuple> > (\
					expression_builder<tuple>::build_expression(np->get_children()[0], context)\
				));\
			} else if constexpr(std::is_same_v<decltype(t), const init_list_type&>)  {\
				return expression_ptr(std::make_unique<tostring_expression<R, initializer_list> > (\
					expression_builder<initializer_list>::build_expression(np->get_children()[0], context)\
				));\
			}\
		}, *np->get_children()[0]->get_type_id());

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

#define CHECK_INDEX_OPERATION(T, A)\
		case node_operation::index:\
			{\
				const tuple_type* tt = std::get_if<tuple_type>(np->get_children()[0]->get_type_id());\
				if (tt) {\
					return expression_ptr(\
						std::make_unique<member_expression<R, A, T> >(\
							expression_builder<A>::build_expression(np->get_children()[0], context),\
							size_t(np->get_children()[1]->get_number())\
						)\
					);\
				} else {\
					const array_type* at = std::get_if<array_type>(np->get_children()[0]->get_type_id());\
					return expression_ptr(\
						std::make_unique<index_expression<R, A, T> >(\
							expression_builder<A>::build_expression(np->get_children()[0], context),\
							expression_builder<number>::build_expression(np->get_children()[1], context),\
							build_default_initialization(at->inner_type_id) \
						)\
					);\
				}\
			}

#define CHECK_CALL_OPERATION(T)\
	case node_operation::call:\
	{\
		std::vector<expression<lvalue>::ptr> arguments;\
		const function_type* ft = std::get_if<function_type>(np->get_children()[0]->get_type_id());\
		for (size_t i = 1; i < np->get_children().size(); ++i) {\
			const node_ptr& child = np->get_children()[i];\
			if (\
				child->is_node_operation() &&\
				std::get<node_operation>(child->get_value()) == node_operation::param\
			) {\
				arguments.push_back(\
					build_lvalue_expression(ft->param_type_id[i-1].type_id, child->get_children()[0], context)\
				);\
			} else {\
				arguments.push_back(\
					expression_builder<lvalue>::build_expression(child, context)\
				);\
			}\
		}\
		return expression_ptr(\
			std::make_unique<call_expression<R, T> >(\
				expression_builder<function>::build_expression(np->get_children()[0], context),\
				std::move(arguments)\
			)\
		);\
	}

		template<typename R>
		class expression_builder{
		private:
			using expression_ptr = typename expression<R>::ptr;
		
			static expression_ptr build_void_expression(const node_ptr& np, compiler_context& context) {
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_BINARY_OPERATION(comma, void, void);
					CHECK_TERNARY_OPERATION(ternary, number, void, void);
					CHECK_CALL_OPERATION(void);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_number_expression(const node_ptr& np, compiler_context& context) {
				if (std::holds_alternative<double>(np->get_value())) {
					return std::make_unique<constant_expression<R, number>>(
						std::get<double>(np->get_value())
					);
				}
				
				CHECK_IDENTIFIER(lnumber);
				
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_UNARY_OPERATION(postinc, lnumber);
					CHECK_UNARY_OPERATION(postdec, lnumber);
					CHECK_UNARY_OPERATION(positive, number);
					CHECK_UNARY_OPERATION(negative, number);
					CHECK_UNARY_OPERATION(bnot, number);
					CHECK_UNARY_OPERATION(lnot, number);
					CHECK_SIZE_OPERATION();
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
					CHECK_COMPARISON_OPERATION(eq);
					CHECK_COMPARISON_OPERATION(ne);
					CHECK_COMPARISON_OPERATION(lt);
					CHECK_COMPARISON_OPERATION(gt);
					CHECK_COMPARISON_OPERATION(le);
					CHECK_COMPARISON_OPERATION(ge);
					CHECK_BINARY_OPERATION(comma, void, number);
					CHECK_BINARY_OPERATION(land, number, number);
					CHECK_BINARY_OPERATION(lor, number, number);
					CHECK_INDEX_OPERATION(number, array);
					CHECK_TERNARY_OPERATION(ternary, number, number, number);
					CHECK_CALL_OPERATION(number);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_lnumber_expression(const node_ptr& np, compiler_context& context) {
				CHECK_IDENTIFIER(lnumber);
				
				switch (std::get<node_operation>(np->get_value())) {
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
					CHECK_INDEX_OPERATION(lnumber, larray);
					CHECK_TERNARY_OPERATION(ternary, number, lnumber, lnumber);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_string_expression(const node_ptr& np, compiler_context& context) {
				if (std::holds_alternative<std::string>(np->get_value())) {
					return std::make_unique<constant_expression<R, string>>(
						std::make_shared<std::string>(std::get<std::string>(np->get_value()))
					);
				}
				
				CHECK_IDENTIFIER(lstring);
				
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_TO_STRING_OPERATION();
					CHECK_BINARY_OPERATION(concat, string, string);
					CHECK_BINARY_OPERATION(comma, void, string);
					CHECK_INDEX_OPERATION(string, array);
					CHECK_TERNARY_OPERATION(ternary, number, string, string);
					CHECK_CALL_OPERATION(string);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_lstring_expression(const node_ptr& np, compiler_context& context) {
				CHECK_IDENTIFIER(lstring);
				
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_BINARY_OPERATION(assign, lstring, string);
					CHECK_BINARY_OPERATION(concat_assign, lstring, string);
					CHECK_BINARY_OPERATION(comma, void, lstring);
					CHECK_INDEX_OPERATION(lstring, larray);
					CHECK_TERNARY_OPERATION(ternary, number, lstring, lstring);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_array_expression(const node_ptr& np, compiler_context& context) {
				CHECK_IDENTIFIER(larray);
				
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_BINARY_OPERATION(comma, void, array);
					CHECK_INDEX_OPERATION(array, array);
					CHECK_TERNARY_OPERATION(ternary, number, array, array);
					CHECK_CALL_OPERATION(array);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_larray_expression(const node_ptr& np, compiler_context& context) {
				CHECK_IDENTIFIER(larray);
				
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_BINARY_OPERATION(assign, larray, array);
					CHECK_BINARY_OPERATION(comma, void, larray);
					CHECK_INDEX_OPERATION(larray, larray);
					CHECK_TERNARY_OPERATION(ternary, number, larray, larray);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_function_expression(const node_ptr& np, compiler_context& context) {
				CHECK_IDENTIFIER(lfunction);
				CHECK_FUNCTION();
				
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_BINARY_OPERATION(comma, void, function);
					CHECK_INDEX_OPERATION(function, array);
					CHECK_TERNARY_OPERATION(ternary, number, function, function);
					CHECK_CALL_OPERATION(function);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_lfunction_expression(const node_ptr& np, compiler_context& context) {
				CHECK_IDENTIFIER(lfunction);
				
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_BINARY_OPERATION(assign, lfunction, function);
					CHECK_BINARY_OPERATION(comma, void, lfunction);
					CHECK_INDEX_OPERATION(lfunction, larray);
					CHECK_TERNARY_OPERATION(ternary, number, lfunction, lfunction);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_tuple_expression(const node_ptr& np, compiler_context& context) {
				CHECK_IDENTIFIER(ltuple);
				
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_BINARY_OPERATION(comma, void, tuple);
					CHECK_INDEX_OPERATION(tuple, array);
					CHECK_TERNARY_OPERATION(ternary, number, tuple, tuple);
					CHECK_CALL_OPERATION(tuple);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_ltuple_expression(const node_ptr& np, compiler_context& context) {
				CHECK_IDENTIFIER(ltuple);
				
				switch (std::get<node_operation>(np->get_value())) {
					CHECK_BINARY_OPERATION(assign, ltuple, tuple);
					CHECK_BINARY_OPERATION(comma, void, ltuple);
					CHECK_INDEX_OPERATION(ltuple, larray);
					CHECK_TERNARY_OPERATION(ternary, number, ltuple, ltuple);
					CHECK_CALL_OPERATION(ltuple);
					default:
						throw expression_builder_error();
				}
			}
			
			static expression_ptr build_initializer_list_expression(const node_ptr& np, compiler_context& context) {
				switch (std::get<node_operation>(np->get_value())) {
					case node_operation::init:
						{
							std::vector<expression<lvalue>::ptr> exprs;
							exprs.reserve(np->get_children().size());
							for (const node_ptr& child : np->get_children()) {
								exprs.emplace_back(build_lvalue_expression(child->get_type_id(), child, context));
							}
							return std::make_unique<init_expression<R> >(std::move(exprs));
						}
					CHECK_BINARY_OPERATION(comma, void, initializer_list);
					CHECK_TERNARY_OPERATION(ternary, number, initializer_list, initializer_list);
					default:
						throw expression_builder_error();
				}
			}
		public:
			static expression_ptr build_expression(const node_ptr& np, compiler_context& context) {
				return std::visit([&](const auto& t) {
					if constexpr(std::is_same_v<decltype(t), const simple_type&>) {
						switch (t) {
							case simple_type::number:
								if (np->is_lvalue()) {
									RETURN_EXPRESSION_OF_TYPE(lnumber);
								} else {
									RETURN_EXPRESSION_OF_TYPE(number);
								}
							case simple_type::string:
								if (np->is_lvalue()) {
									RETURN_EXPRESSION_OF_TYPE(lstring);
								} else {
									RETURN_EXPRESSION_OF_TYPE(string);
								}
							case simple_type::nothing:
								RETURN_EXPRESSION_OF_TYPE(void);
						}
						assert(0);
						return expression_ptr(); //cannot happen
					} else if constexpr(std::is_same_v<decltype(t), const function_type&>) {
						if (np->is_lvalue()) {
							RETURN_EXPRESSION_OF_TYPE(lfunction);
						} else {
							RETURN_EXPRESSION_OF_TYPE(function);
						}
					} else if constexpr(std::is_same_v<decltype(t), const array_type&>) {
						if (np->is_lvalue()) {
							RETURN_EXPRESSION_OF_TYPE(larray);
						} else {
							RETURN_EXPRESSION_OF_TYPE(array);
						}
					} else if constexpr(std::is_same_v<decltype(t), const tuple_type&>) {
						if (np->is_lvalue()) {
							RETURN_EXPRESSION_OF_TYPE(ltuple);
						} else {
							RETURN_EXPRESSION_OF_TYPE(tuple);
						}
					} else if constexpr(std::is_same_v<decltype(t), const init_list_type&>) {
						RETURN_EXPRESSION_OF_TYPE(initializer_list);
					}
				}, *np->get_type_id());
			}
			
			static expression<lvalue>::ptr build_param_expression(const node_ptr& np, compiler_context& context) {
				return std::make_unique<param_expression<R> >(
					expression_builder<R>::build_expression(np, context)
				);
			}
		};

#undef CHECK_CALL_OPERATION
#undef CHECK_INDEX_OPERATION
#undef CHECK_COMPARISON_OPERATION
#undef CHECK_TERNARY_OPERATION
#undef CHECK_BINARY_OPERATION
#undef CHECK_TO_STRING_OPERATION
#undef CHECK_SIZE_OPERATION
#undef CHECK_UNARY_OPERATION
#undef CHECK_FUNCTION
#undef CHECK_IDENTIFIER
#undef RETURN_EXPRESSION_OF_TYPE

		expression<lvalue>::ptr build_lvalue_expression(type_handle type_id, const node_ptr& np, compiler_context& context) {
			return std::visit([&](const auto& t){
				if constexpr(std::is_same_v<decltype(t), const simple_type&>) {
					switch (t) {
						case simple_type::number:
							return expression_builder<number>::build_param_expression(np, context);
						case simple_type::string:
							return expression_builder<string>::build_param_expression(np, context);
						case simple_type::nothing:
							throw expression_builder_error();
					}
					assert(0);
					return expression<lvalue>::ptr(); //cannot happen
				} else if constexpr(std::is_same_v<decltype(t), const function_type&>) {
					return expression_builder<function>::build_param_expression(np, context);
				} else if constexpr(std::is_same_v<decltype(t), const array_type&>) {
					return expression_builder<array>::build_param_expression(np, context);
				} else if constexpr(std::is_same_v<decltype(t), const tuple_type&>) {
					return expression_builder<tuple>::build_param_expression(np, context);
				} else if constexpr(std::is_same_v<decltype(t), const init_list_type&>) {
					throw expression_builder_error();
					return expression<lvalue>::ptr();
				}
			}, *type_id);
		}
		
        class empty_expression: public expression<void> {
            void evaluate(runtime_context&) const override {
            }
        };
        
		template<typename R>
		typename expression<R>::ptr build_expression(type_handle type_id, compiler_context& context, tokens_iterator& it, bool allow_comma) {
			size_t line_number = it->get_line_number();
			size_t char_index = it->get_char_index();
			
			try {
				node_ptr np = parse_expression_tree(context, it, type_id, allow_comma);
				
				if constexpr(std::is_same<void, R>::value) {
					if (!np) {
						return std::make_unique<empty_expression>();
					}
				}
				if constexpr(std::is_same<R, lvalue>::value) {
					return build_lvalue_expression(
						type_id,
						np,
						context
					);
				} else {
					return expression_builder<R>::build_expression(
						np,
						context
					);
				}
			} catch (const expression_builder_error&) {
				throw compiler_error("Expression building failed", line_number, char_index);
			}
		}
		
		
		template <typename T>
		class default_initialization_expression: public expression<lvalue> {
		public:
			lvalue evaluate(runtime_context &context) const override {
				return std::make_shared<variable_impl<T> >(T{});
			}
		};
	}

	expression<void>::ptr build_void_expression(compiler_context& context, tokens_iterator& it) {
		return build_expression<void>(type_registry::get_void_handle(), context, it, true);
	}
	
	expression<number>::ptr build_number_expression(compiler_context& context, tokens_iterator& it) {
		return build_expression<number>(type_registry::get_number_handle(), context, it, true);
	}
	
	expression<lvalue>::ptr build_initialization_expression(
		compiler_context& context,
		tokens_iterator& it,
		type_handle type_id,
		bool allow_comma
	) {
		return build_expression<lvalue>(type_id, context, it, allow_comma);
	}

	expression<lvalue>::ptr build_default_initialization(type_handle type_id) {
		return std::visit([](const auto& t){
			if constexpr(std::is_same_v<decltype(t), const simple_type&>){
				switch (t) {
					case simple_type::number:
						return expression<lvalue>::ptr(std::make_unique<default_initialization_expression<number> >());
					case simple_type::string:
						return expression<lvalue>::ptr(std::make_unique<default_initialization_expression<string> >());
					case simple_type::nothing:
						assert(0);
						return expression<lvalue>::ptr(nullptr); //cannot happen
				}
				assert(0);
				return expression<lvalue>::ptr(nullptr); //cannot happen
			} else if constexpr(std::is_same_v<decltype(t), const function_type&>){
				return expression<lvalue>::ptr(std::make_unique<default_initialization_expression<function> >());
			} else if constexpr(std::is_same_v<decltype(t), const array_type&>){
				return expression<lvalue>::ptr(std::make_unique<default_initialization_expression<array> >());
			} else if constexpr(std::is_same_v<decltype(t), const tuple_type&>){
				std::vector<expression<lvalue>::ptr> exprs;
				
				exprs.reserve(t.inner_type_id.size());
				
				for (type_handle it : t.inner_type_id) {
					exprs.emplace_back(build_default_initialization(it));
				}
				
				return expression<lvalue>::ptr(
					std::make_unique<tuple_initialization_expression>(std::move(exprs))
				);
			} else if constexpr(std::is_same_v<decltype(t), const init_list_type&>){
				//cannot happen
				assert(0);
				return expression<lvalue>::ptr();
			}
		}, *type_id);
	}
}
