#ifndef types_h
#define types_h
#include <vector>
#include <variant>
#include <set>

namespace stork {
	enum struct simple_type {
		nothing,
		number,
		string,
	};
	
	struct array_type {
		int inner_type_id;
	};
	
	struct function_type;
	using type=std::variant<simple_type, array_type, function_type>;
	
	using type_handle = const type*;
	
	struct function_type {
		struct param {
			type_handle type_id;
			bool by_ref;
		};
		int return_type_id;
		std::vector<param> param_type_id;
	};

	class type_registry {
	private:
		struct types_less{
			bool operator()(const type& t1, const type& t2) const;
		};
		std::set<type, types_less> _types;
	public:
		type_registry();
		
		type_handle get_handle(const type& t);
	};
}

#endif /* types_h */
