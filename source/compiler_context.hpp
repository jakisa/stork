#ifndef compiler_context_hpp
#define compiler_context_hpp

#include <unordered_map>
#include <memory>
#include <string>

#include "types.hpp"

namespace stork {
	class variable_info {
	private:
		int _type_id;
		size_t _index;
		bool _is_global;
	public:
		variable_info(int type_id, size_t index, bool is_global);
		
		int type_id() const;
		
		size_t index() const;
		
		bool is_global() const;
	};
	
	class variable_lookup {
	protected:
		std::unordered_map<std::string, variable_info> _variables;
	public:
		virtual const variable_info* find(const std::string& name) const;
		
		virtual void create_variable(std::string name, int type_id) = 0;
		
		virtual ~variable_lookup();
	};
	
	class global_variable_lookup: public variable_lookup {
	public:
		void create_variable(std::string name, int type_id) override;
	};
	
	class local_variable_lookup: public variable_lookup {
	private:
		std::unique_ptr<local_variable_lookup> _parent;
		int _next_variable_index;
	public:
		local_variable_lookup(std::unique_ptr<local_variable_lookup> parent_lookup);
		
		const variable_info* find(const std::string& name) const override;

		void create_variable(std::string name, int type_id) override;
		
		std::unique_ptr<local_variable_lookup> detach_parent();
	};
	
	class function_variable_lookup: public local_variable_lookup {
	private:
		int _next_param_index;
	public:
		function_variable_lookup();
		
		void create_param(std::string name, int type_id);
	};
	
	class compiler_context {
	private:
		global_variable_lookup _globals;
		function_variable_lookup* _params;
		std::unique_ptr<local_variable_lookup> _locals;
		type_registry _types;
	public:
		compiler_context();
		
		int register_type(const type& t);
		
		const type& get_type(int type_id) const;
		
		const variable_info* find(const std::string& name) const;
		
		void create_variable(std::string name, int type_id);
		
		void create_param(std::string name, int type_id);
		
		void enter_scope();
		
		void enter_function();
		
		bool leave_scope();
	};
}

#endif /*compiler_context_hpp*/
