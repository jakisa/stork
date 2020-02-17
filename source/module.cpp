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
		std::unique_ptr<runtime_context> _context;
	public:
		module_impl(){
		}
		
		runtime_context* get_runtime_context() {
			return _context.get();
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
			
			_context = std::make_unique<runtime_context>(compile(it, _external_functions));
		}
		
		bool try_load(const char* path, std::ostream* err) {
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
	
	runtime_context* module::get_runtime_context() {
		return _impl->get_runtime_context();
	}
	
	void module::add_external_function_impl(std::string declaration, function f) {
		_impl->add_external_function_impl(std::move(declaration), std::move(f));
	}
	
	void module::load(const char* path) {
		_impl->load(path);
	}
	
	bool module::try_load(const char* path, std::ostream* err) {
		return _impl->try_load(path, err);
	}
	
	void module::reset_globals() {
		_impl->reset_globals();
	}
	
	module::~module() {
	}
}
