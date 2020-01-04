#ifndef runtime_context_hpp
#define runtime_context_hpp
#include <variant>
#include <vector>
#include <deque>

namespace stork {
	using variable = std::variant<double, std::string>;

	class runtime_context{
	private:
		std::vector<variable> _globals;
		std::deque<variable> _stack;
		std::stack<size_t> _retval_idx;
	public:
		runtime_context(size_t globals) :
			_globals(globals),
			_stack(1)
		{
			_retval_idx.push(0);
		}
	
		variable& global(size_t idx) {
			return _globals[idx];
		}

		variable& retval() {
			return _stack(_retval_idx.top());
		}

		variable& local(size_t idx) {
			return _stack[_retval_idx.top() + 1 + idx];
		}

		variable& param(size_t idx) {
			return _stack[_retval_idx.top() - 1 - idx];
		}
		
		void begin_scope(size_t scope_vars) {
			_stack.resize(_stack.size() + scope_vars);
		}
		
		void end_scope(size_t scope_vars) {
			_stack.resize(_stack.size() - scope_vars);
		}
		
		void push_param(variable v) {
			_stack.push_back(std::move(v));
		}
		
		void call() {
			_retval_idx.push(_stack.size());
			_stack.resize(_retval_idx + 1);
		}
		
		variable end_function(size_t params) {
			variable ret = std::move(_stack[_retval_idx.top()]);
			_stack.resize(_retval_idx.top());
			_retval_idx.pop();
			return ret;
		}
	};
}

#endif /*runtime_context_hpp*/
