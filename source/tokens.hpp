#ifndef tokens_hpp
#define tokens_hpp

#include <optional>
#include <string_view>
#include <ostream>
#include <variant>

namespace stork {
	enum struct reserved_token {
		inc,
		dec,
		
		add,
		sub,
		concat,
		mul,
		div,
		idiv,
		mod,
		
		bitwise_not,
		bitwise_and,
		bitwise_or,
		bitwise_xor,
		shiftl,
		shiftr,
		
		assign,
		
		add_assign,
		sub_assign,
		concat_assign,
		mul_assign,
		div_assign,
		idiv_assign,
		mod_assign,
		
		and_assign,
		or_assign,
		xor_assign,
		shiftl_assign,
		shiftr_assign,
		
		logical_not,
		logical_and,
		logical_or,
		
		eq,
		ne,
		lt,
		gt,
		le,
		ge,
		
		question,
		colon,
		
		comma,
		
		semicolon,
		
		open_round,
		close_round,
		
		open_curly,
		close_curly,
		
		open_square,
		close_square,
		
		kw_sizeof,
		kw_tostring,
		
		kw_if,
		kw_else,
		kw_elif,

		kw_switch,
		kw_case,
		kw_default,

		kw_for,
		kw_while,
		kw_do,

		kw_break,
		kw_continue,
		kw_return,

		kw_function,
		
		kw_void,
		kw_number,
		kw_string,
		
		kw_public,
	};
	
	class push_back_stream;
	
	std::ostream& operator<<(std::ostream& os, reserved_token t);
	
	std::optional<reserved_token> get_keyword(std::string_view word);
	
	std::optional<reserved_token> get_operator(push_back_stream& stream);
	
	struct identifier{
		std::string name;
	};
	
	bool operator==(const identifier& id1, const identifier& id2);
	bool operator!=(const identifier& id1, const identifier& id2);
	
	struct eof{
	};
	
	bool operator==(const eof&, const eof&);
	bool operator!=(const eof&, const eof&);

	using token_value = std::variant<reserved_token, identifier, double, std::string, eof>;

	class token {
	private:
		token_value _value;
		size_t _line_number;
		size_t _char_index;
	public:
		token(token_value value, size_t line_number, size_t char_index);
		
		bool is_reserved_token() const;
		bool is_identifier() const;
		bool is_number() const;
		bool is_string() const;
		bool is_eof() const;
		
		reserved_token get_reserved_token() const;
		const identifier& get_identifier() const;
		double get_number() const;
		const std::string& get_string() const;
		const token_value& get_value() const;
		
		size_t get_line_number() const;
		size_t get_char_index() const;

		bool has_value(const token_value& value) const;
	};
}

namespace std {
	std::string to_string(stork::reserved_token t);
	std::string to_string(const stork::token_value& t);}

#endif /* tokens_hpp */
