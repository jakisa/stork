#include "compiler_context.hpp"

namespace stork{
	variable_info::variable_info(type_handle type_id, size_t index, bool is_global) :
		_type_id(type_id),
		_index(index),
		_is_global(is_global)
	{
	}
	
	type_handle variable_info::type_id() const {
		return _type_id;
	}
	
	size_t variable_info::index() const {
		return _index;
	}
	
	bool variable_info::is_global() const {
		return _is_global;
	}

	const variable_info* variable_lookup::find(const std::string& name) const {
		if (auto it = _variables.find(name); it != _variables.end()) {
			return &it->second;
		} else {
			return nullptr;
		}
	}
	
	variable_lookup::~variable_lookup() {
	}

	void global_variable_lookup::create_variable(std::string name, type_handle type_id) {
		_variables.emplace(std::move(name), variable_info(type_id, _variables.size(), true));
	}

	local_variable_lookup::local_variable_lookup(std::unique_ptr<local_variable_lookup> parent_lookup) :
		_parent(std::move(parent_lookup)),
		_next_variable_index(_parent ? _parent->_next_variable_index : 1)
	{
	}
	
	const variable_info* local_variable_lookup::find(const std::string& name) const {
		if (const variable_info* ret = variable_lookup::find(name)) {
			return ret;
		} else {
			return _parent ? _parent->find(name) : nullptr;
		}
	}

	void local_variable_lookup::create_variable(std::string name, type_handle type_id) {
		_variables.emplace(std::move(name), variable_info(type_id, _next_variable_index++, false));
	}
	
	std::unique_ptr<local_variable_lookup> local_variable_lookup::detach_parent() {
		return std::move(_parent);
	}

	function_variable_lookup::function_variable_lookup() :
		local_variable_lookup(nullptr),
		_next_param_index(-1)
	{
	}
	
	void function_variable_lookup::create_param(std::string name, type_handle type_id) {
		_variables.emplace(std::move(name), variable_info(type_id, _next_param_index--, false));
	}

	compiler_context::compiler_context() :
		_params(nullptr)
	{
	}
	
	const type* compiler_context::get_handle(const type& t) {
		return _types.get_handle(t);
	}
	
	const variable_info* compiler_context::find(const std::string& name) const {
		if (const variable_info* ret = _locals->find(name)) {
			return ret;
		}
		return _globals.find(name);
	}
	
	void compiler_context::create_variable(std::string name, type_handle type_id) {
		if (_locals) {
			_locals->create_variable(std::move(name), type_id);
		} else {
			_globals.create_variable(std::move(name), type_id);
		}
	}
	
	void compiler_context::create_param(std::string name, type_handle type_id) {
		_params->create_param(name, type_id);
	}
	
	void compiler_context::enter_scope() {
		_locals = std::make_unique<local_variable_lookup>(std::move(_locals));
	}
	
	void compiler_context::enter_function() {
		std::unique_ptr<function_variable_lookup> params =  std::make_unique<function_variable_lookup>();
		_params = params.get();
		_locals = std::move(params);
	}
	
	bool compiler_context::leave_scope() {
		if (!_locals) {
			return false;
		}
		
		if (_params == _locals.get()) {
			_params = nullptr;
		}
		
		_locals = _locals->detach_parent();
		
		return true;
	}
}
