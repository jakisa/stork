#include "compiler.hpp"
#include "errors.hpp"
#include "compiler_context.hpp"
#include "expression.hpp"
#include "incomplete_function.hpp"
#include "tokenizer.hpp"
#include "runtime_context.hpp"

namespace stork {
	namespace {
		error unexpected_syntax(const tokens_iterator& it) {
			return unexpected_syntax_error(std::to_string(it->get_value()), it->get_line_number(), it->get_char_index());
		}
		
		//TODO:
		expression<lvalue>::ptr compile_variable_declaration(compiler_context& ctx, tokens_iterator& it) {
			return {};
		}
		
		statement_ptr compile_simple_statement(compiler_context& ctx, tokens_iterator& it);
		
		statement_ptr compile_block_statement(compiler_context& ctx, tokens_iterator& it);
		
		statement_ptr compile_for_statement(compiler_context& ctx, tokens_iterator& it);
		
		statement_ptr compile_while_statement(compiler_context& ctx, tokens_iterator& it);
		
		statement_ptr compile_do_statement(compiler_context& ctx, tokens_iterator& it);
		
		statement_ptr compile_if_statement(compiler_context& ctx, tokens_iterator& it);
		
		statement_ptr compile_switch_statement(compiler_context& ctx, tokens_iterator& it);
		
		statement_ptr compile_var_statement(compiler_context& ctx, tokens_iterator& it);
		
		std::pair<statement_ptr, bool> compile_statement(compiler_context& ctx, tokens_iterator& it) {
			if (it->is_reserved_token()) {
				switch (it->get_reserved_token()) {
					case reserved_token::kw_for:
						return {compile_for_statement(ctx, it), false};
					case reserved_token::kw_while:
						return {compile_while_statement(ctx, it), false};
					case reserved_token::kw_do:
						return {compile_do_statement(ctx, it), false};
					case reserved_token::kw_if:
						return {compile_if_statement(ctx, it), false};
					case reserved_token::kw_switch:
						return {compile_switch_statement(ctx, it), false};
					case reserved_token::kw_var:
						return {compile_var_statement(ctx, it), true};
					default:
						break;
				}
			}
			
			if (it->has_value(reserved_token::open_curly)) {
				return {compile_block_statement(ctx, it), false};
			}
			
			return {compile_simple_statement(ctx, it), false};
		}
		
		statement_ptr compile_simple_statement(compiler_context& ctx, tokens_iterator& it) {
			statement_ptr ret = create_simple_statement(build_void_expression(ctx, it));
			parse_token_value(ctx, it, reserved_token::semicolon);
			return ret;
		}
		
		//TODO:
		statement_ptr compile_for_statement(compiler_context& ctx, tokens_iterator& it) {
			return nullptr;
		}
		
		//TODO:
		statement_ptr compile_while_statement(compiler_context& ctx, tokens_iterator& it) {
			return nullptr;
		}
		
		//TODO:
		statement_ptr compile_do_statement(compiler_context& ctx, tokens_iterator& it) {
			return nullptr;
		}
		
		//TODO:
		statement_ptr compile_if_statement(compiler_context& ctx, tokens_iterator& it) {
			return nullptr;
		}
		
		//TODO:
		statement_ptr compile_switch_statement(compiler_context& ctx, tokens_iterator& it) {
			return nullptr;
		}
		
		//TODO:
		statement_ptr compile_var_statement(compiler_context& ctx, tokens_iterator& it) {
			return nullptr;
		}
		
		std::pair<std::vector<statement_ptr>, size_t> compile_block_contents(compiler_context& ctx, tokens_iterator& it) {
			size_t block_declarations = 0;
			std::vector<statement_ptr> statements;
			
			if (it->has_value(reserved_token::open_curly)) {
				parse_token_value(ctx, it, reserved_token::open_curly);
				
				while (!it->has_value(reserved_token::close_curly)) {
					std::pair<statement_ptr, bool> statement = compile_statement(ctx, it);
					if (statement.second) {
						++block_declarations;
					}
					statements.push_back(std::move(statement.first));
				}
				
				parse_token_value(ctx, it, reserved_token::close_curly);
			} else {
				std::pair<statement_ptr, bool> statement = compile_statement(ctx, it);
				if (statement.second) {
					++block_declarations;
				}
				statements.push_back(std::move(statement.first));
			}
			
			return std::make_pair(std::move(statements), block_declarations);
		}
		
		statement_ptr compile_block_statement(compiler_context& ctx, tokens_iterator& it) {
			std::pair<std::vector<statement_ptr>, size_t> block = compile_block_contents(ctx, it);
			return create_block_statement(std::move(block.first), block.second);
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
	
	shared_block_statement_ptr compile_function_block(compiler_context& ctx, tokens_iterator& it) {
		std::pair<std::vector<statement_ptr>, size_t> block = compile_block_contents(ctx, it);
		return create_shared_block_statement(std::move(block.first), block.second);
	}
	
	runtime_context compile(compiler_context& ctx, tokens_iterator& it) {
		if (!std::holds_alternative<reserved_token>(it->get_value())) {
			throw unexpected_syntax(it);
		}
		
		std::vector<expression<lvalue>::ptr> initializers;
		
		std::vector<incomplete_function> incomplete_functions;
		std::unordered_map<std::string, size_t> public_functions;
		
		while (it) {
			bool public_function = false;
			
			switch (it->get_reserved_token()) {
				case reserved_token::kw_var:
					initializers.push_back(compile_variable_declaration(ctx, it));
					break;
				case reserved_token::kw_public:
					public_function = true;
					if (!(++it)->has_value(reserved_token::kw_function)) {
						throw unexpected_syntax(it);
					}
				case reserved_token::kw_function:
					incomplete_functions.emplace_back(ctx, it);
					if (public_function) {
						public_functions.emplace(incomplete_functions.back().get_name(), incomplete_functions.size() - 1);
					}
					break;
				default:
					throw unexpected_syntax(it);
					
			}
		}
		
		std::vector<lfunction> functions;
		functions.reserve(incomplete_functions.size());
		
		for (incomplete_function& f : incomplete_functions) {
			functions.emplace_back(std::make_shared<variable_impl<function> >(f.compile(ctx)));
		}
		
		return runtime_context(std::move(initializers), std::move(functions), std::move(public_functions));
	}
}
