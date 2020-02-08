#ifndef compiler_hpp
#define compiler_hpp

#include "types.hpp"
#include "tokenizer.hpp"
#include "statement.hpp"
#include "runtime_context.hpp"

namespace stork {
	class compiler_context;
	
	runtime_context compile(compiler_context& ctx, tokens_iterator& it);
	
	type_handle parse_type(compiler_context& ctx, tokens_iterator& it);

	std::string parse_declaration_name(compiler_context& ctx, tokens_iterator& it);
	
	void parse_token_value(compiler_context& ctx, tokens_iterator& it, const token_value& value);
	
	shared_block_statement_ptr compile_function_block(compiler_context& ctx, tokens_iterator& it);
}

#endif /* compiler_hpp */
