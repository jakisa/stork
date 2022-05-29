#include "module.hpp"
#include <vector>
#include <cstdio>
#include "errors.hpp"
#include "push_back_stream.hpp"
#include "tokenizer.hpp"
#include "compiler.hpp"

namespace stork {
	namespace {
		class file{
			file(const file&) = delete;
			void operator=(const file&) = delete;
		private:
			FILE* _fp;
		public:
			file(const char* path):
				_fp(fopen(path, "rt"))
			{
				if (!_fp) {
					throw file_not_found(std::string("'") + path + "' not found");
				}
			}
			
			~file() {
				if (_fp) {
					fclose(_fp);
				}
			}
			
			int operator()() {
				return fgetc(_fp);
			}
		};
	}

	class module_impl {
	private:
		std::vector<std::pair<std::string, function> > _external_functions;
		std::vector<std::string> _public_declarations;
		std::unordered_map<std::string, std::shared_ptr<function> > _public_functions;
		std::unique_ptr<runtime_context> _context;
	public:
		module_impl(){
		}
		
		runtime_context* get_runtime_context() {
			return _context.get();
		}
		
		void add_public_function_declaration(std::string declaration, std::string name, std::shared_ptr<function> fptr) {
			_public_declarations.push_back(std::move(declaration));
			_public_functions.emplace(std::move(name), std::move(fptr));
		}
		
		void add_external_function_impl(std::string declaration, function f) {
			_external_functions.emplace_back(std::move(declaration), std::move(f));
		}
		
		void load(const char* path) {
			file f(path);
			get_character get = [&](){
				return f();
			};
			push_back_stream stream(&get);
			
			tokens_iterator it(stream);
			
			_context = std::make_unique<runtime_context>(compile(it, _external_functions, _public_declarations));
			
			for (const auto& p : _public_functions) {
				*p.second = _context->get_public_function(p.first.c_str());
			}
		}
		
		bool try_load(const char* path, std::ostream* err) noexcept{
			try {
				load(path);
				return true;
			} catch(const file_not_found& e) {
				if (err) {
					*err << e.what() << std::endl;
				}
			} catch(const error& e) {
				if (err) {
					file f(path);
					format_error(
						e,
						[&](){
							return f();
						},
						*err
					);
				}
			} catch(const runtime_error& e) {
				if (err) {
					*err << e.what() << std::endl;
				}
			}
			return false;
		}
		
		void reset_globals() {
			if (_context) {
				_context->initialize();
			}
		}
	};
	
	stork_module::stork_module():
		_impl(std::make_unique<module_impl>())
	{
	}
	
	runtime_context* stork_module::get_runtime_context() {
		return _impl->get_runtime_context();
	}
	
	void stork_module::add_external_function_impl(std::string declaration, function f) {
		_impl->add_external_function_impl(std::move(declaration), std::move(f));
	}

	void stork_module::add_public_function_declaration(std::string declaration, std::string name, std::shared_ptr<function> fptr) {
		_impl->add_public_function_declaration(std::move(declaration), std::move(name), std::move(fptr));
	}
	
	void stork_module::load(const char* path) {
		_impl->load(path);
	}
	
	bool stork_module::try_load(const char* path, std::ostream* err) noexcept{
		return _impl->try_load(path, err);
	}
	
	void stork_module::reset_globals() {
		_impl->reset_globals();
	}
	
	stork_module::~stork_module() {
	}
}
