#include "incomplete_function.hpp"
#include "compiler.hpp"
#include "compiler_context.hpp"
#include "errors.hpp"

namespace stork {
	incomplete_function::incomplete_function(compiler_context& ctx, tokens_iterator& it) {
		function_type ft;
		
		ft.return_type_id = parse_type(ctx, it);
		
		std::string name = parse_declaration_name(ctx, it);
		parse_token_value(ctx, it, reserved_token::open_round);
		
		while(!it->has_value(reserved_token::close_round)) {
			if (!_params.empty()) {
				parse_token_value(ctx, it, reserved_token::comma);
			}
			
			type_handle t = parse_type(ctx, it);
			bool byref = false;
			if (it->has_value(reserved_token::bitwise_and)) {
				byref = true;
				++it;
			}
			ft.param_type_id.push_back({t, byref});
			_params.push_back(parse_declaration_name(ctx, it));
		}
		++it;
		
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
		
		_ft = ctx.get_handle(ft);
		
		ctx.create_identifier(std::move(name), _ft, true);
	}
	
	function incomplete_function::compile(compiler_context& ctx) {
		ctx.enter_scope();
		
		const function_type* ft = std::get_if<function_type>(_ft);
		
		for (int i = 0; i < int(_params.size()); ++i) {
			ctx.create_param(std::move(_params[i]), ft->param_type_id[i].type_id);
		}
		
		tokens_iterator it(_tokens);
		
		shared_statement_ptr stmt = compile_function_block(ctx, it);
		
		return [stmt=std::move(stmt)] (runtime_context& ctx) {
			stmt->execute(ctx);
		};
	}
}
