#include "expression_tree_parser.hpp"
#include "expression_tree.hpp"
#include "tokenizer.hpp"
#include "errors.hpp"
#include <stack>

namespace stork {
	namespace {
		enum struct operator_precedence {
			brackets,
			postfix,
			prefix,
			multiplication,
			addition,
			shift,
			comparison,
			equality,
			bitwise_and,
			bitwise_xor,
			bitwise_or,
			logical_and,
			logical_or,
			assignment,
			comma
		};
		
		enum struct operator_associativity {
			left_to_right,
			right_to_left
		};
		
		struct operator_info {
			node_operation operation;
			operator_precedence precedence;
			operator_associativity associativity;
			int number_of_operands;
			size_t line_number;
			size_t char_index;
			
			operator_info(node_operation operation, size_t line_number, size_t char_index) :
				operation(operation),
				line_number(line_number),
				char_index(char_index)
			{
				switch (operation) {
					case node_operation::init:
						precedence = operator_precedence::brackets;
						break;
					case node_operation::param: // This will never happen. Used only for the node creation.
					case node_operation::postinc:
					case node_operation::postdec:
					case node_operation::index:
					case node_operation::call:
						precedence = operator_precedence::postfix;
						break;
					case node_operation::preinc:
					case node_operation::predec:
					case node_operation::positive:
					case node_operation::negative:
					case node_operation::bnot:
					case node_operation::lnot:
					case node_operation::size:
					case node_operation::tostring:
						precedence = operator_precedence::prefix;
						break;
					case node_operation::mul:
					case node_operation::div:
					case node_operation::idiv:
					case node_operation::mod:
						precedence = operator_precedence::multiplication;
						break;
					case node_operation::add:
					case node_operation::sub:
					case node_operation::concat:
						precedence = operator_precedence::addition;
						break;
					case node_operation::bsl:
					case node_operation::bsr:
						precedence = operator_precedence::shift;
						break;
					case node_operation::lt:
					case node_operation::gt:
					case node_operation::le:
					case node_operation::ge:
						precedence = operator_precedence::comparison;
						break;
					case node_operation::eq:
					case node_operation::ne:
						precedence = operator_precedence::equality;
						break;
					case node_operation::band:
						precedence = operator_precedence::bitwise_and;
						break;
					case node_operation::bxor:
						precedence = operator_precedence::bitwise_xor;
						break;
					case node_operation::bor:
						precedence = operator_precedence::bitwise_or;
						break;
					case node_operation::land:
						precedence = operator_precedence::logical_and;
						break;
					case node_operation::lor:
						precedence = operator_precedence::logical_or;
						break;
					case node_operation::assign:
					case node_operation::add_assign:
					case node_operation::sub_assign:
					case node_operation::mul_assign:
					case node_operation::div_assign:
					case node_operation::idiv_assign:
					case node_operation::mod_assign:
					case node_operation::band_assign:
					case node_operation::bor_assign:
					case node_operation::bxor_assign:
					case node_operation::bsl_assign:
					case node_operation::bsr_assign:
					case node_operation::concat_assign:
					case node_operation::ternary:
						precedence = operator_precedence::assignment;
						break;
					case node_operation::comma:
						precedence = operator_precedence::comma;
						break;
				}
				
				switch (precedence) {
					case operator_precedence::prefix:
					case operator_precedence::assignment:
						associativity = operator_associativity::right_to_left;
						break;
					default:
						associativity = operator_associativity::left_to_right;
						break;
				}
				
				switch (operation) {
					case node_operation::init:
						number_of_operands = 0; //zero or more
						break;
					case node_operation::postinc:
					case node_operation::postdec:
					case node_operation::preinc:
					case node_operation::predec:
					case node_operation::positive:
					case node_operation::negative:
					case node_operation::bnot:
					case node_operation::lnot:
					case node_operation::size:
					case node_operation::tostring:
					case node_operation::call: //at least one
						number_of_operands = 1;
						break;
					case node_operation::ternary:
						number_of_operands = 3;
						break;
					default:
						number_of_operands = 2;
						break;
				}
			}
		};

		operator_info get_operator_info(reserved_token token, bool prefix, size_t line_number, size_t char_index) {
			switch(token) {
				case reserved_token::inc:
					return prefix ? operator_info(node_operation::preinc, line_number, char_index)
					              : operator_info(node_operation::postinc, line_number, char_index);
				case reserved_token::dec:
					return prefix ? operator_info(node_operation::predec, line_number, char_index)
					              : operator_info(node_operation::postdec, line_number, char_index);
				case reserved_token::add:
					return prefix ? operator_info(node_operation::positive, line_number, char_index)
								  : operator_info(node_operation::add, line_number, char_index);
				case reserved_token::sub:
					return prefix ? operator_info(node_operation::negative, line_number, char_index)
					              : operator_info(node_operation::sub, line_number, char_index);
				case reserved_token::concat:
					return operator_info(node_operation::concat, line_number, char_index);
				case reserved_token::mul:
					return operator_info(node_operation::mul, line_number, char_index);
				case reserved_token::div:
					return operator_info(node_operation::div, line_number, char_index);
				case reserved_token::idiv:
					return operator_info(node_operation::idiv, line_number, char_index);
				case reserved_token::mod:
					return operator_info(node_operation::mod, line_number, char_index);
				case reserved_token::bitwise_not:
					return operator_info(node_operation::bnot, line_number, char_index);
				case reserved_token::bitwise_and:
					return operator_info(node_operation::band, line_number, char_index);
				case reserved_token::bitwise_or:
					return operator_info(node_operation::bor, line_number, char_index);
				case reserved_token::bitwise_xor:
					return operator_info(node_operation::bxor, line_number, char_index);
				case reserved_token::shiftl:
					return operator_info(node_operation::bsl, line_number, char_index);
				case reserved_token::shiftr:
					return operator_info(node_operation::bsr, line_number, char_index);
				case reserved_token::assign:
					return operator_info(node_operation::assign, line_number, char_index);
				case reserved_token::add_assign:
					return operator_info(node_operation::add_assign, line_number, char_index);
				case reserved_token::sub_assign:
					return operator_info(node_operation::sub_assign, line_number, char_index);
				case reserved_token::concat_assign:
					return operator_info(node_operation::concat_assign, line_number, char_index);
				case reserved_token::mul_assign:
					return operator_info(node_operation::mod_assign, line_number, char_index);
				case reserved_token::div_assign:
					return operator_info(node_operation::div_assign, line_number, char_index);
				case reserved_token::idiv_assign:
					return operator_info(node_operation::idiv_assign, line_number, char_index);
				case reserved_token::mod_assign:
					return operator_info(node_operation::mod_assign, line_number, char_index);
				case reserved_token::and_assign:
					return operator_info(node_operation::band_assign, line_number, char_index);
				case reserved_token::or_assign:
					return operator_info(node_operation::bor_assign, line_number, char_index);
				case reserved_token::xor_assign:
					return operator_info(node_operation::bxor_assign, line_number, char_index);
				case reserved_token::shiftl_assign:
					return operator_info(node_operation::bsl_assign, line_number, char_index);
				case reserved_token::shiftr_assign:
					return operator_info(node_operation::bsr_assign, line_number, char_index);
				case reserved_token::logical_not:
					return operator_info(node_operation::lnot, line_number, char_index);
				case reserved_token::logical_and:
					return operator_info(node_operation::land, line_number, char_index);
				case reserved_token::logical_or:
					return operator_info(node_operation::lor, line_number, char_index);
				case reserved_token::eq:
					return operator_info(node_operation::eq, line_number, char_index);
				case reserved_token::ne:
					return operator_info(node_operation::ne, line_number, char_index);
				case reserved_token::lt:
					return operator_info(node_operation::lt, line_number, char_index);
				case reserved_token::gt:
					return operator_info(node_operation::gt, line_number, char_index);
				case reserved_token::le:
					return operator_info(node_operation::le, line_number, char_index);
				case reserved_token::ge:
					return operator_info(node_operation::ge, line_number, char_index);
				case reserved_token::question:
					return operator_info(node_operation::ternary, line_number, char_index);
				case reserved_token::comma:
					return operator_info(node_operation::comma, line_number, char_index);
				case reserved_token::open_round:
					return operator_info(node_operation::call, line_number, char_index);
				case reserved_token::open_square:
					return operator_info(node_operation::index, line_number, char_index);
				case reserved_token::kw_sizeof:
					return operator_info(node_operation::size, line_number, char_index);
				case reserved_token::kw_tostring:
					return operator_info(node_operation::tostring, line_number, char_index);
				case reserved_token::open_curly:
					return operator_info(node_operation::init, line_number, char_index);
				default:
					throw unexpected_syntax_error(std::to_string(token), line_number, char_index);
			}
		}

		bool is_end_of_expression(const token& t, bool allow_comma) {
			if (t.is_eof()) {
				return true;
			}

			if (t.is_reserved_token()) {
				switch (t.get_reserved_token()) {
					case reserved_token::semicolon:
					case reserved_token::close_round:
					case reserved_token::close_square:
					case reserved_token::close_curly:
					case reserved_token::colon:
						return true;
					case reserved_token::comma:
						return !allow_comma;
					default:
						return false;
				}
			}
			
			return false;
		}
		
		bool is_evaluated_before(const operator_info& l, const operator_info& r) {
			return l.associativity == operator_associativity::left_to_right ? l.precedence <= r.precedence : l.precedence < r.precedence;
		}
		
		void pop_one_operator(
			std::stack<operator_info>& operator_stack, std::stack<node_ptr>& operand_stack,
			compiler_context& context, size_t line_number, size_t char_index
		) {
			if (operand_stack.size() < operator_stack.top().number_of_operands) {
				throw compiler_error("Failed to parse an expression", line_number, char_index);
			}
			
			std::vector<node_ptr> operands;
			operands.resize(operator_stack.top().number_of_operands);
			
			if (operator_stack.top().precedence != operator_precedence::prefix) {
				operator_stack.top().line_number = operand_stack.top()->get_line_number();
				operator_stack.top().char_index = operand_stack.top()->get_char_index();
			}
			
			for (int i = operator_stack.top().number_of_operands - 1; i >= 0; --i) {
				operands[i] = std::move(operand_stack.top());
				operand_stack.pop();
			}
			
			operand_stack.push(std::make_unique<node>(
				context, operator_stack.top().operation, std::move(operands), operator_stack.top().line_number, operator_stack.top().char_index)
			);
			
			operator_stack.pop();
		}
		
		node_ptr parse_expression_tree_impl(compiler_context& context, tokens_iterator& it, bool allow_comma, bool allow_empty) {
			std::stack<node_ptr> operand_stack;
			std::stack<operator_info> operator_stack;
			
			bool expected_operand = true;
			
			for (; !is_end_of_expression(*it, allow_comma); ++it) {
				if (it->is_reserved_token()) {
					operator_info oi = get_operator_info(
						it->get_reserved_token(), expected_operand, it->get_line_number(), it->get_char_index()
					);
					
					if (oi.operation == node_operation::call && expected_operand) {
						//open round bracket is misinterpreted as a function call
						++it;
						operand_stack.push(parse_expression_tree_impl(context, it, true, false));
						if (it->has_value(reserved_token::close_round)) {
							expected_operand = false;
							continue;
						} else {
							throw syntax_error("Expected closing ')'", it->get_line_number(), it->get_char_index());
						}
					}
					
					if (oi.operation == node_operation::init && expected_operand) {
						++it;
						std::vector<node_ptr> children;
						if (!it->has_value(reserved_token::close_curly)) {
							while (true) {
								children.push_back(parse_expression_tree_impl(context, it, false, false));
								if (it->has_value(reserved_token::close_curly)) {
									break;
								} else if (it->has_value(reserved_token::comma)) {
									++it;
								} else {
									throw syntax_error("Expected ',', or closing '}'", it->get_line_number(), it->get_char_index());
								}
							}
						}
						operand_stack.push(std::make_unique<node>(
							context,
							node_operation::init,
							std::move(children),
							it->get_line_number(),
							it->get_char_index()
						));
						
						expected_operand = false;
						continue;
					}
					
					if ((oi.precedence == operator_precedence::prefix) != expected_operand) {
						throw unexpected_syntax_error(
							std::to_string(it->get_value()),
							it->get_line_number(),
							it->get_char_index()
						);
					}
					
					if (!operator_stack.empty() && is_evaluated_before(operator_stack.top(), oi)) {
						pop_one_operator(operator_stack, operand_stack, context, it->get_line_number(), it->get_char_index());
					}
					
					switch (oi.operation) {
						case node_operation::call:
							++it;
							if (!it->has_value(reserved_token::close_round)) {
								while (true) {
									bool remove_lvalue = !it->has_value(reserved_token::bitwise_and);
									if (!remove_lvalue) {
										++it;
									}
									node_ptr argument = parse_expression_tree_impl(context, it, false, false);
									if (remove_lvalue) {
										size_t line_number = argument->get_line_number();
										size_t char_index = argument->get_char_index();
										std::vector<node_ptr> argument_vector;
										argument_vector.push_back(std::move(argument));
										argument = std::make_unique<node>(
											context,
											node_operation::param,
											std::move(argument_vector),
											line_number,
											char_index
										);
									} else if (!argument->is_lvalue()) {
										throw wrong_type_error(
											std::to_string(argument->get_type_id()),
											std::to_string(argument->get_type_id()),
											true,
											argument->get_line_number(),
											argument->get_char_index()
										);
									}
									
									operand_stack.push(std::move(argument));

									++oi.number_of_operands;
									
									if (it->has_value(reserved_token::close_round)) {
										break;
									} else if (it->has_value(reserved_token::comma)) {
										++it;
									} else {
										throw syntax_error("Expected ',', or closing ')'", it->get_line_number(), it->get_char_index());
									}
								}
							}
							break;
						case node_operation::index:
							++it;
							operand_stack.push(parse_expression_tree_impl(context, it, true, false));
							if (!it->has_value(reserved_token::close_square)) {
								throw syntax_error("Expected closing ]'", it->get_line_number(), it->get_char_index());
							}
							break;
						case node_operation::ternary:
							++it;
							operand_stack.push(parse_expression_tree_impl(context, it, true, false));
							if (!it->has_value(reserved_token::colon)) {
								throw syntax_error("Expected ':'", it->get_line_number(), it->get_char_index());
							}
							break;
						default:
							break;
					}
					
					operator_stack.push(oi);
					
					expected_operand = (oi.precedence != operator_precedence::postfix);
				} else {
					if (!expected_operand) {
						throw unexpected_syntax_error(
							std::to_string(it->get_value()),
							it->get_line_number(),
							it->get_char_index()
						);
					}
					if (it->is_number()) {
						operand_stack.push(std::make_unique<node>(
							context, it->get_number(), std::vector<node_ptr>(), it->get_line_number(), it->get_char_index())
						);
					} else if (it->is_string()) {
						operand_stack.push(std::make_unique<node>(
							context, it->get_string(), std::vector<node_ptr>(), it->get_line_number(), it->get_char_index())
						);
					} else {
						operand_stack.push(std::make_unique<node>(
							context, it->get_identifier(), std::vector<node_ptr>(), it->get_line_number(), it->get_char_index())
						);
					}
					expected_operand = false;
				}
			}
			
			if (expected_operand) {
				if (allow_empty && operand_stack.empty() && operator_stack.empty()) {
					return node_ptr();
				} else {
					throw syntax_error("Operand expected", it->get_line_number(), it->get_char_index());
				}
			}
			
			while(!operator_stack.empty()) {
				pop_one_operator(operator_stack, operand_stack, context, it->get_line_number(), it->get_char_index());
			}
			
			if (operand_stack.size() != 1 || !operator_stack.empty()) {
				throw compiler_error("Failed to parse an expression", it->get_line_number(), it->get_char_index());
			}
			
			return std::move(operand_stack.top());
		}
	}
	
	node_ptr parse_expression_tree(
		compiler_context& context, tokens_iterator& it, type_handle type_id, bool allow_comma
	) {
		node_ptr ret = parse_expression_tree_impl(context, it, allow_comma, type_id == type_registry::get_void_handle());
		if (ret) {
			ret->check_conversion(type_id, false);
		}
		return ret;
	}
}
