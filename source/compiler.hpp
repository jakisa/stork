#ifndef compiler_hpp
#define compiler_hpp

#include "types.hpp"
#include "tokens.hpp"
#include "statement.hpp"

#include <vector>
#include <functional>

namespace stork {
	class compiler_context;
	class tokens_iterator;
	class runtime_context;
	
	using function = std::function<void(runtime_context&)>;

	runtime_context compile(
		tokens_iterator& it,
		const std::vector<std::pair<std::string, function> >& external_functions,
		std::vector<std::string> public_declarations
	);
	
	type_handle parse_type(compiler_context& ctx, tokens_iterator& it);

	std::string parse_declaration_name(compiler_context& ctx, tokens_iterator& it);
	
	void parse_token_value(compiler_context& ctx, tokens_iterator& it, const token_value& value);
	
	shared_statement_ptr compile_function_block(compiler_context& ctx, tokens_iterator& it, type_handle return_type_id);
}

#endif /* compiler_hpp */
