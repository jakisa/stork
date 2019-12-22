#ifndef tokenizer_hpp
#define tokenizer_hpp

#include <functional>
#include <string_view>
#include <iostream>

#include "tokens.hpp"

namespace stork {
	using reserved_token_callback = std::function<void(reserved_token)>;
	using identifier_callback = std::function<void(std::string_view)>;
	using number_callback = std::function<void(double)>;
	using string_callback = std::function<void(std::string_view)>;
	
	void tokenize(
		const get_character& input,
		const reserved_token_callback& on_reserved_token,
		const identifier_callback& on_identifier,
		const number_callback& on_number,
		const string_callback& on_string
	);
}


#endif /* tokenizer_hpp */
