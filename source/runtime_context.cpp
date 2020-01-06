#include "runtime_context.hpp"

namespace stork {
	runtime_context::runtime_context(size_t globals) :
		_globals(globals),
		_stack(1)
	{
		_retval_idx.push(0);
	}

	variable_ptr& runtime_context::global(int idx) {
		return _globals[idx];
	}

	variable_ptr& runtime_context::retval() {
		return _stack[_retval_idx.top()];
	}

	variable_ptr& runtime_context::local(int idx) {
		return _stack[_retval_idx.top() + idx];
	}
	
	void runtime_context::push(variable_ptr v) {
		_stack.push_back(std::move(v));
	}
	
	void runtime_context::end_scope(size_t scope_vars) {
		_stack.resize(_stack.size() - scope_vars);
	}
	
	void runtime_context::call() {
		_retval_idx.push(_stack.size());
		_stack.resize(_retval_idx.top() + 1);
	}
	
	variable_ptr runtime_context::end_function(size_t params) {
		variable_ptr ret = std::move(_stack[_retval_idx.top()]);
		_stack.resize(_retval_idx.top());
		_retval_idx.pop();
		return ret;
	}
}
