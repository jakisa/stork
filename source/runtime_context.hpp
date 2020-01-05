#ifndef runtime_context_hpp
#define runtime_context_hpp
#include <variant>
#include <vector>
#include <deque>
#include <stack>
#include <string>

namespace stork {
	class runtime_context;

	using function = std::function<void(runtime_context&)>;
	using variable = std::variant<double, std::string, function>;

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
	
		variable& global(int idx) {
			return _globals[idx];
		}

		variable& retval() {
			return _stack[_retval_idx.top()];
		}

		variable& local(int idx) {
			return _stack[_retval_idx.top() + idx];
		}
		
		void push(variable v) {
			_stack.push_back(std::move(v));
		}
		
		void end_scope(size_t scope_vars) {
			_stack.resize(_stack.size() - scope_vars);
		}
		
		void call() {
			_retval_idx.push(_stack.size());
			_stack.resize(_retval_idx.top() + 1);
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
