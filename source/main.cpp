#include <iostream>
#include <sstream>
#include "tokenizer.hpp"
#include "errors.hpp"
#include "compiler_context.hpp"
#include "expression_tree.hpp"
#include "expression_tree_parser.hpp"
#include "helpers.hpp"

// When debugging in Xcode, setting the breakpoint will send eof to stdin. We need to ignore it.
// #define XCODE_DEBUG_HACK

namespace {
	using namespace stork;

	std::ostream& operator<<(std::ostream& stream, const node_ptr& node) {
		std::visit(overloaded{
			[&](double d) {
				stream << d;
			},
			[&](const std::string& str) {
				stream << str;
			},
			[&](const identifier& id) {
				stream << id.name;
			},
			[&](node_operation op) {
				switch (op) {
					case node_operation::param:
						stream << node->get_children()[0];
						break;
					case node_operation::preinc:
						stream << "(++" << node->get_children()[0] << ")";
						break;
					case node_operation::predec:
						stream << "(--" << node->get_children()[0] << ")";
						break;
					case node_operation::postinc:
						stream << "(" << node->get_children()[0] << "++)";
						break;
					case node_operation::postdec:
						stream << "(" << node->get_children()[0] << "--)";
						break;
					case node_operation::positive:
						stream << "(+" << node->get_children()[0] << ")";
						break;
					case node_operation::negative:
						stream << "(-" << node->get_children()[0] << ")";
						break;
					case node_operation::bnot:
						stream << "(~" << node->get_children()[0] << ")";
						break;
					case node_operation::lnot:
						stream << "(!" << node->get_children()[0] << ")";
						break;
					case node_operation::add:
						stream << "(" << node->get_children()[0] << "+" << node->get_children()[1] << ")";
						break;
					case node_operation::sub:
						stream << "(" << node->get_children()[0] << "-" << node->get_children()[1] << ")";
						break;
					case node_operation::mul:
						stream << "(" << node->get_children()[0] << "*" << node->get_children()[1] << ")";
						break;
					case node_operation::div:
						stream << "(" << node->get_children()[0] << "/" << node->get_children()[1] << ")";
						break;
					case node_operation::idiv:
						stream << "(" << node->get_children()[0] << "\\" << node->get_children()[1] << ")";
						break;
					case node_operation::mod:
						stream << "(" << node->get_children()[0] << "%" << node->get_children()[1] << ")";
						break;
					case node_operation::band:
						stream << "(" << node->get_children()[0] << "&" << node->get_children()[1] << ")";
						break;
					case node_operation::bor:
						stream << "(" << node->get_children()[0] << "|" << node->get_children()[1] << ")";
						break;
					case node_operation::bxor:
						stream << "(" << node->get_children()[0] << "^" << node->get_children()[1] << ")";
						break;
					case node_operation::bsl:
						stream << "(" << node->get_children()[0] << "<<" << node->get_children()[1] << ")";
						break;
					case node_operation::bsr:
						stream << "(" << node->get_children()[0] << ">>" << node->get_children()[1] << ")";
						break;
					case node_operation::concat:
						stream << "(" << node->get_children()[0] << ".." << node->get_children()[1] << ")";
						break;
					case node_operation::assign:
						stream << "(" << node->get_children()[0] << "=" << node->get_children()[1] << ")";
						break;
					case node_operation::add_assign:
						stream << "(" << node->get_children()[0] << "+=" << node->get_children()[1] << ")";
						break;
					case node_operation::sub_assign:
						stream << "(" << node->get_children()[0] << "-=" << node->get_children()[1] << ")";
						break;
					case node_operation::mul_assign:
						stream << "(" << node->get_children()[0] << "*=" << node->get_children()[1] << ")";
						break;
					case node_operation::div_assign:
						stream << "(" << node->get_children()[0] << "/=" << node->get_children()[1] << ")";
						break;
					case node_operation::idiv_assign:
						stream << "(" << node->get_children()[0] << "\\=" << node->get_children()[1] << ")";
						break;
					case node_operation::mod_assign:
						stream << "(" << node->get_children()[0] << "%=" << node->get_children()[1] << ")";
						break;
					case node_operation::band_assign:
						stream << "(" << node->get_children()[0] << "&=" << node->get_children()[1] << ")";
						break;
					case node_operation::bor_assign:
						stream << "(" << node->get_children()[0] << "|=" << node->get_children()[1] << ")";
						break;
					case node_operation::bxor_assign:
						stream << "(" << node->get_children()[0] << "^=" << node->get_children()[1] << ")";
						break;
					case node_operation::bsl_assign:
						stream << "(" << node->get_children()[0] << "<<=" << node->get_children()[1] << ")";
						break;
					case node_operation::bsr_assign:
						stream << "(" << node->get_children()[0] << ">>=" << node->get_children()[1] << ")";
						break;
					case node_operation::concat_assign:
						stream << "(" << node->get_children()[0] << "..=" << node->get_children()[1] << ")";
						break;
					case node_operation::eq:
						stream << "(" << node->get_children()[0] << "==" << node->get_children()[1] << ")";
						break;
					case node_operation::ne:
						stream << "(" << node->get_children()[0] << "!=" << node->get_children()[1] << ")";
						break;
					case node_operation::lt:
						stream << "(" << node->get_children()[0] << "<" << node->get_children()[1] << ")";
						break;
					case node_operation::gt:
						stream << "(" << node->get_children()[0] << ">" << node->get_children()[1] << ")";
						break;
					case node_operation::le:
						stream << "(" << node->get_children()[0] << "<=" << node->get_children()[1] << ")";
						break;
					case node_operation::ge:
						stream << "(" << node->get_children()[0] << ">=" << node->get_children()[1] << ")";
						break;
					case node_operation::comma:
						stream << "(" << node->get_children()[0] << "," << node->get_children()[1] << ")";
						break;
					case node_operation::land:
						stream << "(" << node->get_children()[0] << "&&" << node->get_children()[1] << ")";
						break;
					case node_operation::lor:
						stream << "(" << node->get_children()[0] << "|=" << node->get_children()[1] << ")";
						break;
					case node_operation::index:
						stream << node->get_children()[0] << "[" << node->get_children()[1] << "]";
						break;
					case node_operation::ternary:
						stream << "(" << node->get_children()[0] << "?" << node->get_children()[1] << ":" << node->get_children()[2];
						break;
					case node_operation::call:
						stream << node->get_children()[0] << "(";
						{
							const char* separator = "";
							for (size_t i = 1; i < node->get_children().size(); ++i) {
								stream << separator << (node->get_children()[i]->is_lvalue() ? "&" : "") << node->get_children()[i];
								separator = ",";
							}
						}
						stream << ")";
						break;
				}
			},
			[&](const auto&) {
			}
		}, node->get_value());
		
		return stream;
	}
}

int main() {
	using namespace stork;
	std::cerr << "Enter the expression, or newline to exit." << std::endl;
	std::string line;
	
	compiler_context context;
	context.create_identifier("a", type_registry::get_number_handle(), false);
	context.create_identifier("b", type_registry::get_number_handle(), false);
	context.create_identifier("c", type_registry::get_number_handle(), false);
	context.create_identifier("d", type_registry::get_number_handle(), true);
	context.create_identifier("e", type_registry::get_number_handle(), true);
	context.create_identifier("f", type_registry::get_number_handle(), true);
	
	context.create_identifier("str1", type_registry::get_string_handle(), false);
	context.create_identifier("str2", type_registry::get_string_handle(), false);
	context.create_identifier("str3", type_registry::get_string_handle(), false);
	context.create_identifier("str4", type_registry::get_string_handle(), true);
	context.create_identifier("str5", type_registry::get_string_handle(), true);
	context.create_identifier("str6", type_registry::get_string_handle(), true);
	
	function_type ft1;
	ft1.return_type_id = type_registry::get_number_handle();
	ft1.param_type_id.push_back({type_registry::get_number_handle(), false});
	ft1.param_type_id.push_back({type_registry::get_number_handle(), false});
	
	function_type ft2;
	ft2.return_type_id = type_registry::get_string_handle();
	ft2.param_type_id.push_back({type_registry::get_string_handle(), true});
	ft2.param_type_id.push_back({type_registry::get_string_handle(), false});
	
	context.create_identifier("add", context.get_handle(ft1), true);
	context.create_identifier("concat_to", context.get_handle(ft2), true);
	
	context.create_identifier("numarr", context.get_handle(array_type{type_registry::get_number_handle()}), false);
	context.create_identifier("strarr", context.get_handle(array_type{type_registry::get_string_handle()}), false);
	
	do {
		if (std::cin.eof()){
			std::cin.clear();
		}
		std::getline(std::cin, line);
		if (!line.empty())  {
			std::istringstream strstream(line);
			
			get_character input = [&strstream]() {
				return strstream.get();
			};
			
			try {
				push_back_stream stream(input);
				
				tokens_iterator it(stream);
				
				node_ptr n = parse_expression_tree(context, it, type_registry::get_void_handle(), false, true, false);
				std::cout << "Expression parsed as: " << n << std::endl;
			} catch (const error& err) {
				strstream.clear();
				strstream.seekg(0);
				format_error(err, input, std::cerr);
			}
		}
	}while(
		!line.empty()
#ifdef XCODE_DEBUG_HACK
		|| std::cin.eof()
#endif
	);
	return 0;
}
