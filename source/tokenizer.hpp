#ifndef tokenizer_hpp
#define tokenizer_hpp

#include <functional>
#include <string_view>
#include <iostream>
#include <variant>

#include "tokens.hpp"
#include "push_back_stream.hpp"

namespace stork {
	class tokens_iterator {
	private:
		push_back_stream& _stream;
		token _current;
	public:
		tokens_iterator(push_back_stream& stream);
		
		const token& operator*() const;
		const token* operator->() const;
		
		tokens_iterator& operator++();
		
		explicit operator bool() const;
	};
}


#endif /* tokenizer_hpp */
