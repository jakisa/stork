#include "errors.hpp"
#include <sstream>

namespace stork {
	error::error(std::string message, size_t line_number, size_t char_index) noexcept :
		_message(std::move(message)),
		_line_number(line_number),
		_char_index(char_index)
	{
	}
		
	const char* error::what() const noexcept {
		return _message.c_str();
	}
		
	size_t error::line_number() const noexcept {
		return _line_number;
	}
	
	size_t error::char_index() const noexcept {
		return _char_index;
	}
	
	error parsing_error(std::string_view message, size_t line_number, size_t char_index) {
		std::string error_message("Parsing error: ");
		error_message += message;
		return error(std::move(error_message), line_number, char_index);
	}
	
	error syntax_error(std::string_view message, size_t line_number, size_t char_index) {
		std::string error_message("Syntax error: ");
		error_message += message;
		return error(std::move(error_message), line_number, char_index);
	}
	
	error semantic_error(std::string_view message, size_t line_number, size_t char_index) {
		std::string error_message("Semantic error: ");
		error_message += message;
		return error(std::move(error_message), line_number, char_index);
	}
	
	error compiler_error(std::string_view message, size_t line_number, size_t char_index) {
		std::string error_message("Compiler error: ");
		error_message += message;
		return error(std::move(error_message), line_number, char_index);
	}
	
	error unexpected_error(std::string_view unexpected, size_t line_number, size_t char_index) {
		std::string message("Unexpected '");
		message += unexpected;
		message += "'";
		return parsing_error(message, line_number, char_index);
	}
	
	error unexpected_syntax_error(std::string_view unexpected, size_t line_number, size_t char_index) {
		std::string message("Unexpected '");
		message += unexpected;
		message += "'";
		return syntax_error(message, line_number, char_index);
	}
	
	error expected_syntax_error(std::string_view expected, size_t line_number, size_t char_index) {
		std::string message("Expected '");
		message += expected;
		message += "'";
		return syntax_error(message, line_number, char_index);
	}
	
	error undeclared_error(std::string_view undeclared, size_t line_number, size_t char_index) {
		std::string message("Undeclared identifier '");
		message += undeclared;
		message += "'";
		return semantic_error(message, line_number, char_index);
	}

	error wrong_type_error(std::string_view source, std::string_view destination,
	                       bool lvalue, size_t line_number,
		size_t char_index) {
		std::string message;
		if (lvalue) {
			message += "'";
			message += source;
			message += "' is not a lvalue";
		} else {
			message += "Cannot convert '";
			message +=  source;
			message += "' to '";
			message +=  destination;
			message += "'";
		}
		return semantic_error(message, line_number, char_index);
	}
	
	error already_declared_error(std::string_view name, size_t line_number, size_t char_index) {
		std::string message = "'";
		message += name;
		message += "' is already declared";
		return semantic_error(message, line_number, char_index);
	}

	void format_error(const error& err, const get_character& source, std::ostream& output) {
		output << "(" << (err.line_number() + 1) << ") " << err.what() << std::endl;
		
		size_t char_index = 0;
		
		for (size_t line_number = 0; line_number < err.line_number(); ++char_index) {
			int c = source();
			if (c < 0) {
				return;
			} else if (c == '\n') {
				++line_number;
			}
		}

		size_t index_in_line = err.char_index() - char_index;
		
		std::string line;
		for (size_t idx = 0;; ++idx) {
			int c = source();
			if (c < 0 || c == '\n' || c == '\r') {
				break;
			}
			line += char(c == '\t' ? ' ' : c);
		}
		
		output << line << std::endl;
		
		for (size_t idx = 0; idx < index_in_line; ++idx) {
			output << " ";
		}
		
		output << "^" << std::endl;
	}
	
	runtime_error::runtime_error(std::string message) noexcept:
		_message(std::move(message))
	{
	}
		
	const char* runtime_error::what() const noexcept {
		return _message.c_str();
	}
	
	void runtime_assertion(bool b, const char* message) {
		if (!b) {
			throw runtime_error(message);
		}
	}
	
	file_not_found::file_not_found(std::string message) noexcept:
		_message(std::move(message))
	{
	}
		
	const char* file_not_found::what() const noexcept {
		return _message.c_str();
	}
}
