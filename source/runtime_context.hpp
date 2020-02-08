#ifndef runtime_context_hpp
#define runtime_context_hpp
#include <variant>
#include <vector>
#include <deque>
#include <stack>
#include <string>
#include "variable.hpp"
#include "lookup.hpp"

namespace stork {
	class runtime_context{
	private:
		lookup<std::string, lfunction> _public_functions;
		std::vector<variable_ptr> _globals;
		std::deque<variable_ptr> _stack;
		std::stack<size_t> _retval_idx;
	public:
		runtime_context(size_t globals, std::vector<std::pair<std::string, lfunction> > public_functions);
	
		variable_ptr& global(int idx);

		variable_ptr& retval();

		variable_ptr& local(int idx);
		
		void push(variable_ptr v);
		
		void end_scope(size_t scope_vars);
		
		void call();
		
		variable_ptr end_function(size_t params);
		
		lfunction get_public_function(std::string_view name) const;
	};
}

#endif /*runtime_context_hpp*/
