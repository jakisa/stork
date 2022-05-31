#include "compiler_context.hpp"

namespace stork{
	identifier_info::identifier_info(type_handle type_id, int index, identifier_scope scope) :
		_type_id(type_id),
		_index(index),
		_scope(scope)
	{
	}
	
	type_handle identifier_info::type_id() const {
		return _type_id;
	}
	
	int identifier_info::index() const {
		return _index;
	}
	
	identifier_scope identifier_info::get_scope() const {
		return _scope;
	}

	const identifier_info* identifier_lookup::insert_identifier(std::string name, type_handle type_id, int index, identifier_scope scope) {
		return &_identifiers.emplace(std::move(name), identifier_info(type_id, index, scope)).first->second;
	}
	
	int identifier_lookup::identifiers_size() const {
		return _identifiers.size();
	}

	const identifier_info* identifier_lookup::find(const std::string& name) const {
		if (auto it = _identifiers.find(name); it != _identifiers.end()) {
			return &it->second;
		} else {
			return nullptr;
		}
	}
	
	bool identifier_lookup::can_declare(const std::string& name) const {
		return _identifiers.find(name) == _identifiers.end();
	}
	
	identifier_lookup::~identifier_lookup() {
	}

	const identifier_info* global_variable_lookup::create_identifier(std::string name, type_handle type_id) {
		return insert_identifier(std::move(name), type_id, identifiers_size(), identifier_scope::global_variable);
	}

	local_variable_lookup::local_variable_lookup(std::unique_ptr<local_variable_lookup> parent_lookup) :
		_parent(std::move(parent_lookup)),
		_next_identifier_index(_parent ? _parent->_next_identifier_index : 1)
	{
	}
	
	const identifier_info* local_variable_lookup::find(const std::string& name) const {
		if (const identifier_info* ret = identifier_lookup::find(name)) {
			return ret;
		} else {
			return _parent ? _parent->find(name) : nullptr;
		}
	}

	const identifier_info* local_variable_lookup::create_identifier(std::string name, type_handle type_id) {
		return insert_identifier(std::move(name), type_id, _next_identifier_index++, identifier_scope::local_variable);
	}
	
	std::unique_ptr<local_variable_lookup> local_variable_lookup::detach_parent() {
		return std::move(_parent);
	}

	param_lookup::param_lookup() :
		local_variable_lookup(nullptr),
		_next_param_index(-1)
	{
	}
	
	const identifier_info* param_lookup::create_param(std::string name, type_handle type_id) {
		return insert_identifier(std::move(name), type_id, _next_param_index--, identifier_scope::local_variable);
	}
	
	const identifier_info* function_lookup::create_identifier(std::string name, type_handle type_id) {
		return insert_identifier(std::move(name), type_id, identifiers_size(), identifier_scope::function);
	}

	compiler_context::compiler_context() :
		_params(nullptr)
	{
	}
	
	const type* compiler_context::get_handle(const type& t) {
		return _types.get_handle(t);
	}
	
	const identifier_info* compiler_context::find(const std::string& name) const {
		if (_locals) {
			if (const identifier_info* ret = _locals->find(name)) {
				return ret;
			}
		}
		if (const identifier_info* ret = _functions.find(name)) {
			return ret;
		}
		return _globals.find(name);
	}
	
	const identifier_info* compiler_context::create_identifier(std::string name, type_handle type_id) {
		if (_locals) {
			return _locals->create_identifier(std::move(name), type_id);
		} else {
			return _globals.create_identifier(std::move(name), type_id);
		}
	}
	
	const identifier_info* compiler_context::create_param(std::string name, type_handle type_id) {
		return _params->create_param(name, type_id);
	}
	
	const identifier_info* compiler_context::create_function(std::string name, type_handle type_id) {
		return _functions.create_identifier(name, type_id);
	}
	
	void compiler_context::enter_scope() {
		_locals = std::make_unique<local_variable_lookup>(std::move(_locals));
	}
	
	void compiler_context::enter_function() {
		std::unique_ptr<param_lookup> params = std::make_unique<param_lookup>();
		_params = params.get();
		_locals = std::move(params);
	}
	
	void compiler_context::leave_scope() {
		if (_params == _locals.get()) {
			_params = nullptr;
		}
		
		_locals = _locals->detach_parent();
	}
	
	bool compiler_context::can_declare(const std::string& name) const {
		return _locals ? _locals->can_declare(name) : (_globals.can_declare(name) && _functions.can_declare(name));
	}
	
	compiler_context::scope_raii compiler_context::scope() {
		return scope_raii(*this);
	}
	
	compiler_context::function_raii compiler_context::function() {
		return function_raii(*this);
	}
	
	compiler_context::scope_raii::scope_raii(compiler_context& context):
		_context(context)
	{
		_context.enter_scope();
	}
	
	compiler_context::scope_raii::~scope_raii() {
		_context.leave_scope();
	}
	
	compiler_context::function_raii::function_raii(compiler_context& context):
		_context(context)
	{
		_context.enter_function();
	}
	
	compiler_context::function_raii::~function_raii() {
		_context.leave_scope();
	}
}
