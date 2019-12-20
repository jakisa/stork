#include "tokens.hpp"
#include "lookup.hpp"
#include <string_view>

namespace lightscript {
	namespace {
		const lookup<std::string_view, reserved_token> operator_token_map {
			{"++", reserved_token::inc},
			{"--", reserved_token::dec},
			
			{"+", reserved_token::add},
			{"-", reserved_token::sub},
			{"*", reserved_token::mul},
			{"/", reserved_token::div},
			{"\\", reserved_token::idiv},
			{"%", reserved_token::mod},
			
			{"~", reserved_token::bitwise_not},
			{"&", reserved_token::bitwise_and},
			{"|", reserved_token::bitwise_or},
			{"^", reserved_token::bitwise_xor},
			{"<<", reserved_token::shiftl},
			{">>", reserved_token::shiftr},
			
			{"=", reserved_token::assign},
			
			{"+=", reserved_token::add_assign},
			{"-=", reserved_token::sub_assign},
			{"*=", reserved_token::mul_assign},
			{"/=", reserved_token::div_assign},
			{"\\=", reserved_token::idiv_assign},
			{"%=", reserved_token::mod_assign},
			
			{"&=", reserved_token::and_assign},
			{"|=", reserved_token::or_assign},
			{"^=", reserved_token::xor_assign},
			{"<<=", reserved_token::shiftl_assign},
			{">>=", reserved_token::shiftr_assign},
			
			{"!", reserved_token::logical_not},
			{"&&", reserved_token::logical_and},
			{"||", reserved_token::logical_or},
			
			{"==", reserved_token::eq},
			{"!=", reserved_token::ne},
			{"<", reserved_token::lt},
			{">", reserved_token::gt},
			{"<=", reserved_token::le},
			{">=", reserved_token::ge},
			
			{"?", reserved_token::question},
			{":", reserved_token::colon},
			
			{",", reserved_token::comma},
			
			{";", reserved_token::semicolon},
			
			{"(", reserved_token::open_round},
			{")", reserved_token::close_round},
			
			{"{", reserved_token::open_curly},
			{"}", reserved_token::close_curly},
			
			{"[", reserved_token::open_square},
			{"]", reserved_token::close_square},
		};
		
		const lookup<std::string_view, reserved_token> keyword_token_map {
			{"if", reserved_token::kw_if},
			{"else", reserved_token::kw_else},
			{"elif", reserved_token::kw_elif},

			{"switch", reserved_token::kw_switch},
			{"case", reserved_token::kw_case},
			{"default", reserved_token::kw_default},

			{"for", reserved_token::kw_for},
			{"while", reserved_token::kw_while},
			{"do", reserved_token::kw_do},

			{"break", reserved_token::kw_break},
			{"continue", reserved_token::kw_continue},
			{"return", reserved_token::kw_return},

			{"var", reserved_token::kw_var},
			{"fun", reserved_token::kw_fun},
			
			{"void", reserved_token::kw_void},
			{"number", reserved_token::kw_number},
			{"string", reserved_token::kw_string}
		};
		
		const lookup<reserved_token, std::string_view> token_string_map = ([](){
			std::vector<std::pair<reserved_token, std::string_view>> container;
			container.reserve(operator_token_map.size() + keyword_token_map.size());
			for (const auto& p : operator_token_map) {
				container.emplace_back(p.second, p.first);
			}
			for (const auto& p : keyword_token_map) {
				container.emplace_back(p.second, p.first);
			}
			return lookup<reserved_token, std::string_view>(std::move(container));
		})();
	}
	
	std::ostream& operator<<(std::ostream& os, reserved_token t) {
		os << token_string_map.find(t)->second;
		return os;
	}
	
	std::optional<reserved_token> get_keyword(std::string_view word) {
		auto it = keyword_token_map.find(word);
		return it == keyword_token_map.end() ? std::nullopt : std::make_optional(it->second);
	}
}
