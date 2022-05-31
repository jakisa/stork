#ifndef compiler_context_hpp
#define compiler_context_hpp

#include <unordered_map>
#include <memory>
#include <string>

#include "types.hpp"

namespace stork {

	enum struct identifier_scope {
		global_variable,
		local_variable,
		function,
	};

	class identifier_info {
	private:
		type_handle _type_id;
		int _index;
		identifier_scope _scope;
	public:
		identifier_info(type_handle type_id, int index, identifier_scope scope);
		
		type_handle type_id() const;
		
		int index() const;
		
		identifier_scope get_scope() const;
	};
	
	class identifier_lookup {
	private:
		std::unordered_map<std::string, identifier_info> _identifiers;
	protected:
		const identifier_info* insert_identifier(std::string name, type_handle type_id, int index, identifier_scope scope);
		int identifiers_size() const;
	public:
		virtual const identifier_info* find(const std::string& name) const;
		
		virtual const identifier_info* create_identifier(std::string name, type_handle type_id) = 0;
		
		bool can_declare(const std::string& name) const;
		
		virtual ~identifier_lookup();
	};
	
	class global_variable_lookup: public identifier_lookup {
	public:
		const identifier_info* create_identifier(std::string name, type_handle type_id) override;
	};
	
	class local_variable_lookup: public identifier_lookup {
	private:
		std::unique_ptr<local_variable_lookup> _parent;
		int _next_identifier_index;
	public:
		local_variable_lookup(std::unique_ptr<local_variable_lookup> parent_lookup);
		
		const identifier_info* find(const std::string& name) const override;

		const identifier_info* create_identifier(std::string name, type_handle type_id) override;
		
		std::unique_ptr<local_variable_lookup> detach_parent();
	};
	
	class param_lookup: public local_variable_lookup {
	private:
		int _next_param_index;
	public:
		param_lookup();
		
		const identifier_info* create_param(std::string name, type_handle type_id);
	};
	
	class function_lookup: public identifier_lookup {
	public:
		const identifier_info* create_identifier(std::string name, type_handle type_id) override;
	};
	
	class compiler_context {
	private:
		function_lookup _functions;
		global_variable_lookup _globals;
		param_lookup* _params;
		std::unique_ptr<local_variable_lookup> _locals;
		type_registry _types;
		
		class scope_raii {
		private:
			compiler_context& _context;
		public:
			scope_raii(compiler_context& context);
			~scope_raii();
		};
		
		class function_raii {
		private:
			compiler_context& _context;
		public:
			function_raii(compiler_context& context);
			~function_raii();
		};
		
		void enter_function();
		void enter_scope();
		void leave_scope();
	public:
		compiler_context();
		
		type_handle get_handle(const type& t);
		
		const identifier_info* find(const std::string& name) const;
		
		const identifier_info* create_identifier(std::string name, type_handle type_id);
		
		const identifier_info* create_param(std::string name, type_handle type_id);
		
		const identifier_info* create_function(std::string name, type_handle type_id);
		
		bool can_declare(const std::string& name) const;
		
		scope_raii scope();
		function_raii function();
	};
}

#endif /*compiler_context_hpp*/
