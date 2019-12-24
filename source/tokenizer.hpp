#ifndef tokenizer_hpp
#define tokenizer_hpp

#include <functional>
#include <string_view>
#include <iostream>
#include <variant>

#include "tokens.hpp"
#include "push_back_stream.hpp"

namespace stork {
	token tokenize(push_back_stream& stream);
}


#endif /* tokenizer_hpp */
