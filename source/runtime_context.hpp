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
		std::vector<lfunction> _functions;
		std::vector<variable_ptr> _globals;
		std::deque<variable_ptr> _stack;
		std::stack<size_t> _retval_idx;
	public:
		runtime_context(size_t globals, std::vector<lfunction> functions);
	
		variable_ptr& global(int idx);

		variable_ptr& retval();

		variable_ptr& local(int idx);
		
		const lfunction& get_function(int idx) const;
		
		void push(variable_ptr v);
		
		void end_scope(size_t scope_vars);
		
		void call();
		
		variable_ptr end_function(size_t params);
	};
}

#endif /*runtime_context_hpp*/
