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
			) {
				using next_unpacker = unpacker<R, std::tuple<Unpacked..., Left0>, std::tuple<Left...> >;
				if constexpr(std::is_same<Left0, const std::string&>::value) {
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
					static_assert(std::is_same<Left0, number>::value);
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
			) {
				return std::apply(f, t);
			}
		};

		template<typename R, typename... Args>
		function create_external_function(std::function<R(Args...)> f) {
			static_assert(std::is_same<R, number>::value || std::is_same<R, std::string>::value);
			
			return [f=std::move(f)](runtime_context& ctx) {
				R retval = unpacker<R, std::tuple<>, std::tuple<Args...> >()(ctx, f, std::tuple<>());
				if constexpr(std::is_same<R, std::string>::value) {
					ctx.retval() = std::make_shared<variable_impl<string> >(std::make_shared<std::string>(std::move(retval)));
				} else {
					static_assert(std::is_same<R, number>::value);
					ctx.retval() = std::make_shared<variable_impl<number> >(retval);
				}
			};
		}
	}
	
	class module_impl;
	
	class module {
	private:
		std::unique_ptr<module_impl> _impl;
		void add_external_function_impl(std::string name, function f);
	public:
		module();
		
		template<typename R, typename... Args>
		void add_external_function(const char* name, std::function<R(Args...)> f) {
			add_external_function_impl(name, details::create_external_function(std::move(f)));
		}
		
		void load(const char* path);
		bool try_load(const char* path, std::ostream& err);
		
		void reset_globals();
		
		~module();
	};
}
#endif /* module_hpp */
