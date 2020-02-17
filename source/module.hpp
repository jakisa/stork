#ifndef module_hpp
#define module_hpp

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <iostream>
#include "variable.hpp"
#include "runtime_context.hpp"

namespace stork {
	namespace details {
		template<typename R, typename Unpacked, typename Left>
		struct unpacker;
		
		template<typename R, typename... Unpacked, typename Left0, typename... Left>
		struct unpacker<R, std::tuple<Unpacked...>, std::tuple<Left0, Left...> >{
			R operator()(
				runtime_context& ctx,
				const std::function<R(Unpacked..., Left0, Left...)>& f,
				std::tuple<Unpacked...> t
			) const {
				using next_unpacker = unpacker<R, std::tuple<Unpacked..., Left0>, std::tuple<Left...> >;
				if constexpr(std::is_convertible<const std::string&, Left0>::value) {
					return next_unpacker()(
						ctx,
						f,
						std::tuple_cat(
							std::move(t),
							std::tuple<Left0>(
								ctx.local(
									-1 - int(sizeof...(Unpacked))
								)->static_pointer_downcast<lstring>()->value
							)
						)
					);
				} else {
					static_assert(std::is_convertible<number, Left0>::value);
					return next_unpacker()(
						ctx,
						f,
						std::tuple_cat(
							std::move(t),
							std::tuple<Left0>(
								ctx.local(
									-1 - int(sizeof...(Unpacked))
								)->static_pointer_downcast<lnumber>()->value
							)
						)
					);
				}
			}
		};
	
		template<typename R, typename... Unpacked>
		struct unpacker<R, std::tuple<Unpacked...>, std::tuple<> >{
			R operator()(
				runtime_context& ctx,
				const std::function<R(Unpacked...)>& f,
				std::tuple<Unpacked...> t
			) const {
				return std::apply(f, t);
			}
		};
		
		template<typename R, typename... Args>
		function create_external_function(std::function<R(Args...)> f) {
			return [f=std::move(f)](runtime_context& ctx) {
				if constexpr(std::is_same<R, void>::value) {
					unpacker<R, std::tuple<>, std::tuple<Args...> >()(ctx, f, std::tuple<>());
				} else {
					R retval = unpacker<R, std::tuple<>, std::tuple<Args...> >()(ctx, f, std::tuple<>());
					if constexpr(std::is_convertible<R, std::string>::value) {
						ctx.retval() = std::make_shared<variable_impl<string> >(std::make_shared<std::string>(std::move(retval)));
					} else {
						static_assert(std::is_convertible<R, number>::value);
						ctx.retval() = std::make_shared<variable_impl<number> >(retval);
					}
				}
			};
		}
		
		template<typename T>
		struct retval_declaration{
			static constexpr const char* result() {
				if constexpr(std::is_same<T, void>::value) {
					return "void";
				} else if constexpr(std::is_convertible<T, std::string>::value) {
					return "string";
				} else {
					static_assert(std::is_convertible<T, number>::value);
					return "number";
				}
			}
		};
		
		template<typename T>
		struct argument_declaration{
			static constexpr const char* result() {
				if constexpr(std::is_convertible<const std::string&, T>::value) {
					return "string";
				} else {
					static_assert(std::is_convertible<number,  T>::value);
					return "number";
				}
			}
		};
		
		struct function_argument_string{
			std::string str;
			function_argument_string(const char* p):
				str(p)
			{
			}
			
			function_argument_string& operator+=(const function_argument_string& oth) {
				str += ", ";
				str += oth.str;
				return *this;
			}
		};
		
		template<typename R, typename... Args>
		std::string create_external_function_declaration(const char* name, const std::function<R(Args...)>& f) {
			if constexpr(sizeof...(Args) == 0) {
				return std::string("function ") + retval_declaration<R>::result() + " " + name + "()";
			} else {
				return std::string("function ") + retval_declaration<R>::result() + " " + name +
					"(" +
					(
						function_argument_string(argument_declaration<Args>::result()) += ...
					).str +
					")";
			}
		}
		
		inline variable_ptr to_variable(number n) {
			return std::make_shared<variable_impl<number> >(n);
		}
		
		inline variable_ptr to_variable(std::string str) {
			return std::make_shared<variable_impl<string> >(std::make_shared<std::string>(std::move(str)));
		}
		
	}
	
	class module_impl;
	
	class module {
	private:
		std::unique_ptr<module_impl> _impl;
		void add_external_function_impl(std::string declaration, function f);
		runtime_context* get_runtime_context();
	public:
		module();
		
		template<typename R, typename... Args>
		void add_external_function(const char* name, std::function<R(Args...)> f) {
			add_external_function_impl(
				details::create_external_function_declaration(name, f),
				details::create_external_function(std::move(f))
			);
		}
		
		template<typename R, typename... Args>
		auto create_public_function_caller(const std::string& name) {
			return [this, name](Args... args){
				if constexpr(std::is_same<R, void>::value) {
					get_runtime_context()->call(
						get_runtime_context()->get_public_function(name.c_str()),
						{to_variable(args)...}
					);
				} else {
					return get_runtime_context()->call(
						get_runtime_context()->get_public_function(name.c_str()),
						{to_variable(args)...}
					)->template static_pointer_downcast<std::shared_ptr<variable_impl<R> > >()->value;
				}
			};
		}
		
		void load(const char* path);
		bool try_load(const char* path, std::ostream* err = nullptr);
		
		void reset_globals();
		
		~module();
	};
}
#endif /* module_hpp */
