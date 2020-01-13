#ifndef types_h
#define types_h
#include <vector>
#include <variant>
#include <map>
#include <deque>

namespace stork {
	enum struct simple_type {
		nothing,
		number,
		string,
	};
	
	struct array_type {
		int inner_type_id;
	};
	
	struct function_type {
		struct param {
			int type_id;
			bool by_ref;
		};
		int return_type_id;
		std::vector<param> param_type_id;
	};
	
	using type=std::variant<simple_type, array_type, function_type>;

	class type_registry {
	private:
		struct types_less{
			bool operator()(const type& t1, const type& t2) const;
		};
		std::map<type, int, types_less> _types_map;
		std::deque<type> _types;
	public:
		type_registry();
		
		int register_type(const type& t);
		
		const type& get_type(int type_id) const;
	};
}

#endif /* types_h */
