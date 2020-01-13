#include "types.hpp"

namespace stork {

	bool type_registry::types_less::operator()(const type& t1, const type& t2) const {
		const size_t idx1 = t1.index();
		const size_t idx2 = t2.index();
		
		if (idx1 != idx2) {
			return idx1 < idx2;
		}
		
		switch (idx1) {
			case 0:
				return std::get<0>(t1) < std::get<0>(t2);
			case 1:
				return std::get<1>(t1).inner_type_id < std::get<1>(t2).inner_type_id;
			case 2:
			{
				const function_type& ft1 = std::get<2>(t1);
				const function_type& ft2 = std::get<2>(t2);
				
				if (ft1.return_type_id != ft2.return_type_id) {
					return ft1.return_type_id < ft2.return_type_id;
				}
				
				if (ft1.param_type_id.size() != ft2.param_type_id.size()) {
					return ft1.param_type_id.size() < ft2.param_type_id.size();
				}
				
				for (size_t i = 0; i < ft1.param_type_id.size(); ++i) {
					if (ft1.param_type_id[i].type_id != ft2.param_type_id[i].type_id) {
						return ft1.param_type_id[i].type_id < ft2.param_type_id[i].type_id;
					}
					if (ft1.param_type_id[i].by_ref != ft2.param_type_id[i].by_ref) {
						return ft2.param_type_id[i].by_ref;
					}
				}
			}
		}
		
		return false;
	}

	type_registry::type_registry(){
		register_type(simple_type::nothing);
		register_type(simple_type::number);
		register_type(simple_type::string);
	}
	
	int type_registry::register_type(const type& t) {
		auto pib = _types_map.emplace(t, _types_map.size());
		
		if (pib.second) {
			_types.push_back(t);
		}
		
		return pib.first->second;
	}
	
	const type& type_registry::get_type(int type_id) const {
		return _types[type_id];
	}
}
