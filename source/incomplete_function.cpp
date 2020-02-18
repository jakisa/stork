#include "incomplete_function.hpp"
#include "compiler.hpp"
#include "compiler_context.hpp"
#include "errors.hpp"
#include "tokenizer.hpp"

namespace stork {
	function_declaration parse_function_declaration(compiler_context& ctx, tokens_iterator& it) {
		function_declaration ret;
		
		parse_token_value(ctx, it, reserved_token::kw_function);
		
		function_type ft;
		ft.return_type_id = parse_type(ctx, it);
		ret.name = parse_declaration_name(ctx, it);
		
		{
			auto _ = ctx.function();
			
			parse_token_value(ctx, it, reserved_token::open_round);
			
			while(!it->has_value(reserved_token::close_round)) {
				if (!ret.params.empty()) {
					parse_token_value(ctx, it, reserved_token::comma);
				}
				
				type_handle t = parse_type(ctx, it);
				bool byref = false;
				if (it->has_value(reserved_token::bitwise_and)) {
					byref = true;
					++it;
				}
				ft.param_type_id.push_back({t, byref});
				
				if (!it->has_value(reserved_token::close_round) && !it->has_value(reserved_token::comma)) {
					ret.params.push_back(parse_declaration_name(ctx, it));
				} else {
					ret.params.push_back("@"+std::to_string(ret.params.size()));
				}
			}
			++it;
		}
		
		ret.type_id = ctx.get_handle(ft);
		
		return ret;
	}

	incomplete_function::incomplete_function(compiler_context& ctx, tokens_iterator& it) {
		_decl = parse_function_declaration(ctx, it);
		
		_tokens.push_back(*it);
		
		parse_token_value(ctx, it, reserved_token::open_curly);
		
		int nesting = 1;
		
		while (nesting && !it->is_eof()) {
			if (it->has_value(reserved_token::open_curly)) {
				++nesting;
			}
			
			if (it->has_value(reserved_token::close_curly)) {
				--nesting;
			}
			
			_tokens.push_back(*it);
			++it;
		}
		
		if (nesting) {
			throw unexpected_syntax_error("end of file", it->get_line_number(), it->get_char_index());
		}
		
		ctx.create_function(_decl.name, _decl.type_id);
	}
	
	incomplete_function::incomplete_function(incomplete_function&& orig) noexcept:
		_tokens(std::move(orig._tokens)),
		_decl(std::move(orig._decl))
	{
	}
	
	const function_declaration& incomplete_function::get_decl() const {
		return _decl;
	}
	
	function incomplete_function::compile(compiler_context& ctx) {
		auto _ = ctx.function();
		
		const function_type* ft = std::get_if<function_type>(_decl.type_id);
		
		for (int i = 0; i < int(_decl.params.size()); ++i) {
			ctx.create_param(std::move(_decl.params[i]), ft->param_type_id[i].type_id);
		}
		
		tokens_iterator it(_tokens);
		
		shared_statement_ptr stmt = compile_function_block(ctx, it, ft->return_type_id);
		
		return [stmt=std::move(stmt)] (runtime_context& ctx) {
			stmt->execute(ctx);
		};
	}
}
