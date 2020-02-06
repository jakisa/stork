#include "compiler_context.hpp"

namespace stork{
	identifier_info::identifier_info(type_handle type_id, size_t index, bool is_global, bool is_constant) :
		_type_id(type_id),
		_index(index),
		_is_global(is_global),
		_is_constant(is_constant)
	{
	}
	
	type_handle identifier_info::type_id() const {
		return _type_id;
	}
	
	size_t identifier_info::index() const {
		return _index;
	}
	
	bool identifier_info::is_global() const {
		return _is_global;
	}
	
	bool identifier_info::is_constant() const {
		return _is_constant;
	}

	const identifier_info* identifier_lookup::insert_identifier(std::string name, type_handle type_id, size_t index, bool is_global, bool is_constant) {
		return &_identifiers.emplace(std::move(name), identifier_info(type_id, index, is_global, is_constant)).first->second;
	}
	
	size_t identifier_lookup::identifiers_size() const {
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

	const identifier_info* global_identifier_lookup::create_identifier(std::string name, type_handle type_id, bool is_constant) {
		return insert_identifier(std::move(name), type_id, identifiers_size(), true, is_constant);
	}

	local_identifier_lookup::local_identifier_lookup(std::unique_ptr<local_identifier_lookup> parent_lookup) :
		_parent(std::move(parent_lookup)),
		_next_identifier_index(_parent ? _parent->_next_identifier_index : 1)
	{
	}
	
	const identifier_info* local_identifier_lookup::find(const std::string& name) const {
		if (const identifier_info* ret = identifier_lookup::find(name)) {
			return ret;
		} else {
			return _parent ? _parent->find(name) : nullptr;
		}
	}

	const identifier_info* local_identifier_lookup::create_identifier(std::string name, type_handle type_id, bool is_constant) {
		return insert_identifier(std::move(name), type_id, _next_identifier_index++, false, is_constant);
	}
	
	std::unique_ptr<local_identifier_lookup> local_identifier_lookup::detach_parent() {
		return std::move(_parent);
	}

	function_identifier_lookup::function_identifier_lookup() :
		local_identifier_lookup(nullptr),
		_next_param_index(-1)
	{
	}
	
	const identifier_info* function_identifier_lookup::create_param(std::string name, type_handle type_id) {
		return insert_identifier(std::move(name), type_id, _next_param_index--, false, false);
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
		return _globals.find(name);
	}
	
	const identifier_info* compiler_context::create_identifier(std::string name, type_handle type_id, bool is_constant) {
		if (_locals) {
			return _locals->create_identifier(std::move(name), type_id, is_constant);
		} else {
			return _globals.create_identifier(std::move(name), type_id, is_constant);
		}
	}
	
	const identifier_info* compiler_context::create_param(std::string name, type_handle type_id) {
		return _params->create_param(name, type_id);
	}
	
	void compiler_context::enter_scope() {
		_locals = std::make_unique<local_identifier_lookup>(std::move(_locals));
	}
	
	void compiler_context::enter_function() {
		std::unique_ptr<function_identifier_lookup> params = std::make_unique<function_identifier_lookup>();
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
	
	bool compiler_context::can_declare(const std::string& name) const {
		return _locals ? _locals->can_declare(name) : _globals.can_declare(name);
	}
}
