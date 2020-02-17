#ifndef runtime_context_hpp
#define runtime_context_hpp
#include <variant>
#include <vector>
#include <deque>
#include <stack>
#include <string>
#include <unordered_map>
#include "variable.hpp"
#include "lookup.hpp"
#include "expression.hpp"

namespace stork {
	class runtime_context {
	private:
		std::vector<function> _functions;
		std::unordered_map<std::string, size_t> _public_functions;
		std::vector<expression<lvalue>::ptr> _initializers;
		std::vector<variable_ptr> _globals;
		std::deque<variable_ptr> _stack;
		size_t _retval_idx;
		
		class scope {
		private:
			runtime_context& _context;
			size_t _stack_size;
		public:
			scope(runtime_context& context);
			~scope();
		};
		
	public:
		runtime_context(
			std::vector<expression<lvalue>::ptr> initializers,
			std::vector<function> functions,
			std::unordered_map<std::string, size_t> public_functions
		);
	
		void initialize();

		variable_ptr& global(int idx);
		variable_ptr& retval();
		variable_ptr& local(int idx);

		const function& get_function(int idx) const;
		const function& get_public_function(const char* name) const;

		scope enter_scope();
		void push(variable_ptr v);
		
		variable_ptr call(const function& f, std::vector<variable_ptr> params);
	};
}

#endif /*runtime_context_hpp*/
