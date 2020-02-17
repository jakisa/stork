#ifndef incomplete_function_hpp
#define incomplete_function_hpp

#include "tokens.hpp"
#include "types.hpp"
#include <deque>
#include <functional>

namespace stork {
	class compiler_context;
	class runtime_context;
	class tokens_iterator;
	using function = std::function<void(runtime_context&)>;

	struct function_declaration{
		std::string name;
		type_handle type_id;
		std::vector<std::string> params;
	};
	
	function_declaration parse_function_declaration(compiler_context& ctx, tokens_iterator& it);

	class incomplete_function {
	private:
		function_declaration _decl;
		std::deque<token> _tokens;
		size_t _index;
	public:
		incomplete_function(compiler_context& ctx, tokens_iterator& it);
		
		incomplete_function(incomplete_function&& orig) noexcept;
		
		const function_declaration& get_decl() const;
		
		function compile(compiler_context& ctx);
	};
}

#endif /* incomplete_function_hpp */
