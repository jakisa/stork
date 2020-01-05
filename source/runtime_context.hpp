#ifndef runtime_context_hpp
#define runtime_context_hpp
#include <variant>
#include <vector>
#include <deque>
#include <stack>
#include <string>
#include "variable.hpp"

namespace stork {
	class runtime_context{
	private:
		std::vector<variable_ptr> _globals;
		std::deque<variable_ptr> _stack;
		std::stack<size_t> _retval_idx;
	public:
		runtime_context(size_t globals) :
			_globals(globals),
			_stack(1)
		{
			_retval_idx.push(0);
		}
	
		variable_ptr& global(int idx) {
			return _globals[idx];
		}

		variable_ptr& retval() {
			return _stack[_retval_idx.top()];
		}

		variable_ptr& local(int idx) {
			return _stack[_retval_idx.top() + idx];
		}
		
		void push(variable_ptr v) {
			_stack.push_back(std::move(v));
		}
		
		void end_scope(size_t scope_vars) {
			_stack.resize(_stack.size() - scope_vars);
		}
		
		void call() {
			_retval_idx.push(_stack.size());
			_stack.resize(_retval_idx.top() + 1);
		}
		
		variable_ptr end_function(size_t params) {
			variable_ptr ret = std::move(_stack[_retval_idx.top()]);
			_stack.resize(_retval_idx.top());
			_retval_idx.pop();
			return ret;
		}
	};
}

#endif /*runtime_context_hpp*/
