#ifndef tokenizer_hpp
#define tokenizer_hpp

#include <functional>
#include <string_view>
#include <iostream>
#include <variant>
#include <deque>

#include "tokens.hpp"

namespace stork {
	class push_back_stream;

	class tokens_iterator {
		tokens_iterator(const tokens_iterator&) = delete;
		void operator=(const tokens_iterator&) = delete;
	private:
		std::function<token()> _get_next_token;
		token _current;
	public:
		tokens_iterator(push_back_stream& stream);
		tokens_iterator(std::deque<token>& tokens);
		
		const token& operator*() const;
		const token* operator->() const;
		
		tokens_iterator& operator++();
		
		explicit operator bool() const;
	};
}


#endif /* tokenizer_hpp */
