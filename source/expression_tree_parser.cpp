#include "expression_tree_parser.hpp"
#include "expression_tree.hpp"
#include "tokenizer.hpp"
#include "errors.hpp"
#include <stack>

namespace stork {
	namespace {
		enum struct operator_precedence {
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
			
			operator_info(node_operation operation) :
				operation(operation)
			{
				switch (operation) {
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
					case node_operation::postinc:
					case node_operation::postdec:
					case node_operation::preinc:
					case node_operation::predec:
					case node_operation::positive:
					case node_operation::negative:
					case node_operation::bnot:
					case node_operation::lnot:
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
					return prefix ? operator_info(node_operation::preinc) : operator_info(node_operation::postinc);
				case reserved_token::dec:
					return prefix ? operator_info(node_operation::predec) : operator_info(node_operation::postdec);
				case reserved_token::add:
					return prefix ? operator_info(node_operation::positive) : operator_info(node_operation::add);
				case reserved_token::sub:
					return prefix ? operator_info(node_operation::negative) : operator_info(node_operation::sub);
				case reserved_token::concat:
					return operator_info(node_operation::concat);
				case reserved_token::mul:
					return operator_info(node_operation::mul);
				case reserved_token::div:
					return operator_info(node_operation::div);
				case reserved_token::idiv:
					return operator_info(node_operation::idiv);
				case reserved_token::mod:
					return operator_info(node_operation::mod);
				case reserved_token::bitwise_not:
					return operator_info(node_operation::bnot);
				case reserved_token::bitwise_and:
					return operator_info(node_operation::band);
				case reserved_token::bitwise_or:
					return operator_info(node_operation::bor);
				case reserved_token::bitwise_xor:
					return operator_info(node_operation::bxor);
				case reserved_token::shiftl:
					return operator_info(node_operation::bsl);
				case reserved_token::shiftr:
					return operator_info(node_operation::bsr);
				case reserved_token::assign:
					return operator_info(node_operation::assign);
				case reserved_token::add_assign:
					return operator_info(node_operation::add_assign);
				case reserved_token::sub_assign:
					return operator_info(node_operation::sub_assign);
				case reserved_token::concat_assign:
					return operator_info(node_operation::concat_assign);
				case reserved_token::mul_assign:
					return operator_info(node_operation::mod_assign);
				case reserved_token::div_assign:
					return operator_info(node_operation::div_assign);
				case reserved_token::idiv_assign:
					return operator_info(node_operation::idiv_assign);
				case reserved_token::mod_assign:
					return operator_info(node_operation::mod_assign);
				case reserved_token::and_assign:
					return operator_info(node_operation::band_assign);
				case reserved_token::or_assign:
					return operator_info(node_operation::bor_assign);
				case reserved_token::xor_assign:
					return operator_info(node_operation::bxor_assign);
				case reserved_token::shiftl_assign:
					return operator_info(node_operation::bsl_assign);
				case reserved_token::shiftr_assign:
					return operator_info(node_operation::bsr_assign);
				case reserved_token::logical_not:
					return operator_info(node_operation::lnot);
				case reserved_token::logical_and:
					return operator_info(node_operation::land);
				case reserved_token::logical_or:
					return operator_info(node_operation::lor);
				case reserved_token::eq:
					return operator_info(node_operation::eq);
				case reserved_token::ne:
					return operator_info(node_operation::ne);
				case reserved_token::lt:
					return operator_info(node_operation::lt);
				case reserved_token::gt:
					return operator_info(node_operation::gt);
				case reserved_token::le:
					return operator_info(node_operation::le);
				case reserved_token::ge:
					return operator_info(node_operation::ge);
				case reserved_token::question:
					return operator_info(node_operation::ternary);
				case reserved_token::comma:
					return operator_info(node_operation::comma);
				case reserved_token::open_round:
					return operator_info(node_operation::call);
				case reserved_token::open_square:
					return operator_info(node_operation::index);
				default:
					throw unexpected_error(std::to_string(token), line_number, char_index);
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
	}

	node_ptr parse_expression_tree(compiler_context& context, push_back_stream& stream,
	                               type_handle type_id, bool lvalue, bool allow_comma) {
		
		std::stack<node_ptr> operand_stack;
		std::stack<operator_info> operator_stack;
		
		for (token t = tokenize(stream); !is_end_of_expression(t, allow_comma); t = tokenize(stream)) {
			if (t.is_number()) {
				operand_stack.push(std::make_unique<node>(context, t.get_number(), std::vector<node_ptr>(),
				                                          t.get_line_number(), t.get_char_index()));
			} else if (t.is_string()) {
				operand_stack.push(std::make_unique<node>(context, t.get_string(), std::vector<node_ptr>(),
				                                          t.get_line_number(), t.get_char_index()));
			} else if (t.is_identifier()) {
				operand_stack.push(std::make_unique<node>(context, t.get_identifier(), std::vector<node_ptr>(),
				                                          t.get_line_number(), t.get_char_index()));
			} else {
				
			}
		}
		
		return node_ptr();
	}
}
