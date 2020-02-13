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
	class runtime_context{
	private:
		std::vector<lfunction> _functions;
		std::unordered_map<std::string, size_t> _public_functions;
		std::vector<expression<lvalue>::ptr> _initializers;
		std::vector<variable_ptr> _globals;
		std::deque<variable_ptr> _stack;
		std::stack<size_t> _retval_idx;
	public:
		runtime_context(
			std::vector<expression<lvalue>::ptr> initializers,
			std::vector<lfunction> functions,
			std::unordered_map<std::string, size_t> public_functions
		);
	
		void call_public_function(const std::string& name);
	
		void initialize();
	
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
