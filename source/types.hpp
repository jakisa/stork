#ifndef types_h
#define types_h
#include <vector>
#include <variant>
#include <set>
#include <ostream>

namespace stork {
	enum struct simple_type {
		nothing,
		number,
		string,
	};
	
	struct array_type;
	struct function_type;
	struct tuple_type;
	struct init_list_type;
	
	using type = std::variant<simple_type, array_type, function_type, tuple_type, init_list_type>;
	using type_handle = const type*;
	
	struct array_type {
		type_handle inner_type_id;
	};
	
	struct function_type {
		struct param {
			type_handle type_id;
			bool by_ref;
		};
		type_handle return_type_id;
		std::vector<param> param_type_id;
	};
	
	struct tuple_type {
		std::vector<type_handle> inner_type_id;
	};
	
	struct init_list_type {
		std::vector<type_handle> inner_type_id;
	};
	
	class type_registry {
	private:
		struct types_less{
			bool operator()(const type& t1, const type& t2) const;
		};
		std::set<type, types_less> _types;
		
		static type void_type;
		static type number_type;
		static type string_type;
	public:
		type_registry();
		
		type_handle get_handle(const type& t);
		
		static type_handle get_void_handle() {
			return &void_type;
		}
		
		static type_handle get_number_handle() {
			return &number_type;
		}
		
		static type_handle get_string_handle() {
			return &string_type;
		}
	};
}

namespace std {
	std::string to_string(stork::type_handle t);
}

#endif /* types_h */
