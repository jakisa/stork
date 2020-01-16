#include "types.hpp"
#include "helpers.hpp"

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
	}
	
	type_handle type_registry::get_handle(const type& t) {
		return std::visit(overloaded{
			[](simple_type t) {
				switch (t) {
					case simple_type::nothing:
						return type_registry::get_void_handle();
					case simple_type::number:
						return type_registry::get_number_handle();
					case simple_type::string:
						return type_registry::get_string_handle();
				}
			},
			[this](const auto& t) {
				return &(*(_types.insert(t).first));
			}
		}, t);
	}
	
	type type_registry::void_type = simple_type::nothing;
	type type_registry::number_type = simple_type::number;
	type type_registry::string_type = simple_type::string;
}

namespace std {
	using namespace stork;
	std::string to_string(type_handle t) {
		return std::visit(overloaded{
			[](simple_type st) {
				switch (st) {
					case simple_type::nothing:
						return std::string("void");
					case simple_type::number:
						return std::string("number");
					case simple_type::string:
						return std::string("string");
				}
			},
			[](const array_type& at) {
				std::string ret = to_string(at.inner_type_id);
				ret += "[]";
				return ret;
			},
			[](const function_type& ft) {
				std::string ret = to_string(ft.return_type_id) + "(";
				const char* separator = "";
				for (const function_type::param& p: ft.param_type_id) {
					ret +=  separator + to_string(p.type_id) + (p.by_ref ? "&" : "");
					separator = ",";
				}
				ret += ")";
				return ret;
			}
		}, *t);
	}
}
