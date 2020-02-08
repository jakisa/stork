#include "compiler.hpp"
#include "errors.hpp"
#include "compiler_context.hpp"

namespace stork {
	namespace {
		error unexpected_syntax(const tokens_iterator& it) {
			return unexpected_syntax_error(std::to_string(it->get_value()), it->get_line_number(), it->get_char_index());
		}
	}

	void parse_token_value(compiler_context&, tokens_iterator& it, const token_value& value) {
		if (it->has_value(value)) {
			++it;
			return;
		}
		throw expected_syntax_error(std::to_string(value), it->get_line_number(), it->get_char_index());
	}
	
	std::string parse_declaration_name(compiler_context& ctx, tokens_iterator& it) {
		if (!it->is_identifier()) {
			throw unexpected_syntax(it);
		}

		std::string ret = it->get_identifier().name;
		
		if (!ctx.can_declare(ret)) {
			throw already_declared_error(ret, it->get_line_number(), it->get_char_index());
		}
		
		return ret;
	}

	type_handle parse_type(compiler_context& ctx, tokens_iterator& it) {
		if (!it->is_reserved_token()) {
			throw unexpected_syntax(it);
		}
		
		type_handle t = nullptr;
		
		switch (it->get_reserved_token()) {
			case reserved_token::kw_void:
				t = ctx.get_handle(simple_type::nothing);
				break;
			case reserved_token::kw_number:
				t = ctx.get_handle(simple_type::number);
				break;
			case reserved_token::kw_string:
				t = ctx.get_handle(simple_type::string);
				break;
			default:
				throw unexpected_syntax(it);
		}
		
		while ((++it)->is_reserved_token()) {
			switch (it->get_reserved_token()) {
				case reserved_token::open_square:
					parse_token_value(ctx, ++it, reserved_token::close_square);
					t = ctx.get_handle(array_type{t});
					break;
				case reserved_token::open_round:
					{
						function_type ft;
						ft.return_type_id = t;
						++it;
						while (!it->has_value(reserved_token::close_round)) {
							if (!ft.param_type_id.empty()) {
								parse_token_value(ctx, it, reserved_token::comma);
							}
							type_handle param_type = parse_type(ctx, it);
							if (it->has_value(reserved_token::bitwise_and)) {
								ft.param_type_id.push_back({param_type, true});
								++it;
							} else {
								ft.param_type_id.push_back({param_type, false});
							}
						}
						
						t = ctx.get_handle(ft);
					}
					break;
				default:
					break;
			}
		}
		
		return t;
	}
	
	//TODO:
	shared_block_statement_ptr compile_function_block(compiler_context& ctx, tokens_iterator& it) {
		return {};
	}
	
	//TODO:
	runtime_context compile(compiler_context& ctx, tokens_iterator& it) {
		return runtime_context(0, {});
	}
}
