#ifndef incomplete_function_hpp
#define incomplete_function_hpp

#include "tokenizer.hpp"
#include "types.hpp"
#include <deque>
#include <functional>

namespace stork {
	class compiler_context;
	class runtime_context;
	using function = std::function<void(runtime_context&)>;

	class incomplete_function {
	private:
		std::deque<token> _tokens;
		std::vector<std::string> _params;
		type_handle _ft;
	public:
		incomplete_function(compiler_context& ctx, tokens_iterator& it);
		function compile(compiler_context& ctx);
	};
}

#endif /* incomplete_function_hpp */
