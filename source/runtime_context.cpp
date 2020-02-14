#include "runtime_context.hpp"

namespace stork {
	runtime_context::runtime_context(
		std::vector<expression<lvalue>::ptr> initializers,
		std::vector<lfunction> functions,
		std::unordered_map<std::string, size_t> public_functions
	) :
		_functions(std::move(functions)),
		_public_functions(std::move(public_functions)),
		_initializers(std::move(initializers)),
		_stack(1)
	{
		_retval_idx.push(0);
		
		_globals.reserve(_initializers.size());
		initialize();
	}
	
	void runtime_context::initialize() {
		_globals.clear();
		
		for (const auto& initializer : _initializers) {
			_globals.emplace_back(initializer->evaluate(*this));
		}
	}
	
	void runtime_context::call_public_function(const std::string& name) {
		_functions[_public_functions[name]]->value(*this);
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
	
	const lfunction& runtime_context::get_function(int idx) const {
		return _functions[idx];
	}
	
	void runtime_context::push(variable_ptr v) {
		_stack.push_back(std::move(v));
	}
	
	void runtime_context::call() {
		_retval_idx.push(_stack.size());
		_stack.resize(_retval_idx.top() + 1);
	}
	
	variable_ptr runtime_context::end_function(size_t params) {
		variable_ptr ret = std::move(_stack[_retval_idx.top()]);
		_stack.resize(_retval_idx.top() - params);
		_retval_idx.pop();
		return ret;
	}
	
	runtime_context::scope_raii::scope_raii(size_t stack_size, runtime_context* context):
		_context(context),
		_stack_size(stack_size)
	{
	}
			
	runtime_context::scope_raii::~scope_raii() {
		_context->_stack.resize(_stack_size);
	}
	
	runtime_context::scope_raii runtime_context::enter_scope() {
		return scope_raii(_stack.size(), this);
	}
}
