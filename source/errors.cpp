#include "errors.hpp"

namespace lightscript {
	error::error(std::string message, size_t line_number) noexcept :
		_message(std::move(message)),
		_line_number(line_number)
	{
	}
		
	const char* error::what() const noexcept {
		return _message.c_str();
	}
		
	size_t error::line_number() const noexcept {
		return _line_number;
	}
	
	void throw_parsing_error(const char* message, size_t line_number) {
		std::string error_message("Parsing error: ");
		error_message += message;
		throw error(std::move(error_message), line_number);
	}
	
	void throw_unexpected_error(std::string_view unexpected, size_t line_number) {
		std::string message("Unexpected '");
		message += unexpected;
		message += "'";
		throw_parsing_error(message.c_str(), line_number);
	}
};

