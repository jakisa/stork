#include "runtime_context.hpp"
#include "errors.hpp"

namespace stork {
	runtime_context::runtime_context(
		std::vector<expression<lvalue>::ptr> initializers,
		std::vector<function> functions,
		std::unordered_map<std::string, size_t> public_functions
	) :
		_functions(std::move(functions)),
		_public_functions(std::move(public_functions)),
		_initializers(std::move(initializers)),
		_retval_idx(0)
	{
		_globals.reserve(_initializers.size());
		initialize();
	}
	
	void runtime_context::initialize() {
		_globals.clear();
		
		for (const auto& initializer : _initializers) {
			_globals.emplace_back(initializer->evaluate(*this));
		}
	}
	
	variable_ptr& runtime_context::global(int idx) {
		runtime_assertion(idx < _globals.size(), "Uninitialized global variable access");
		return _globals[idx];
	}

	variable_ptr& runtime_context::retval() {
		return _stack[_retval_idx];
	}

	variable_ptr& runtime_context::local(int idx) {
		return _stack[_retval_idx + idx];
	}
	
	const function& runtime_context::get_function(int idx) const {
		return _functions[idx];
	}
	
	const function& runtime_context::get_public_function(const char* name) const{
		return _functions[_public_functions.find(name)->second];
	}
	
	runtime_context::scope runtime_context::enter_scope() {
		return scope(*this);
	}
	
	void runtime_context::push(variable_ptr v) {
		_stack.push_back(std::move(v));
	}

	variable_ptr runtime_context::call(const function& f, std::vector<variable_ptr> params) {
		for (size_t i = params.size(); i > 0; --i) {
			_stack.push_back(std::move(params[i-1]));
		}
		size_t old_retval_idx = _retval_idx;
		
		_retval_idx = _stack.size();
		_stack.resize(_retval_idx + 1);
		
		runtime_assertion(bool(f), "Uninitialized function call");
		
		f(*this);
		
		variable_ptr ret = std::move(_stack[_retval_idx]);
		
		_stack.resize(_retval_idx - params.size());
		
		_retval_idx = old_retval_idx;
		
		return ret;
	}
	
	runtime_context::scope::scope(runtime_context& context):
		_context(context),
		_stack_size(context._stack.size())
	{
	}
	
	runtime_context::scope::~scope() {
		_context._stack.resize(_stack_size);
	}
}
