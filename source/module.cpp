#include "module.hpp"
#include <vector>
#include "errors.hpp"

namespace stork {

	class module_impl {
	private:
		std::vector<std::pair<std::string, function> > _external_functions;
		std::unique_ptr<runtime_context> _context;
	public:
		module_impl(){
		}
		
		void add_external_function_impl(std::string name, function f) {
			_external_functions.emplace_back(std::move(name), std::move(f));
		}
		
		//TODO:
		void load(const char* path) {
		}
		
		//TODO:
		bool try_load(const char* path, std::ostream& err) {
			try {
				load(path);
				return true;
			} catch(const error& e) {
				
			}
			return false;
		}
		
		void reset_globals() {
			if (_context) {
				_context->initialize();
			}
		}
	};
	
	module::module():
		_impl(std::make_unique<module_impl>())
	{
	}
	
	void module::add_external_function_impl(std::string name, function f) {
		_impl->add_external_function_impl(std::move(name), std::move(f));
	}
	
	void module::load(const char* path) {
		_impl->load(path);
	}
	
	bool module::try_load(const char* path, std::ostream& err) {
		return _impl->try_load(path, err);
	}
	
	void module::reset_globals() {
		_impl->reset_globals();
	}
	
	module::~module() {
	}
}
