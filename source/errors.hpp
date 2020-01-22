#ifndef errors_hpp
#define errors_hpp

#include <exception>
#include <string>
#include <string_view>
#include <ostream>

namespace stork {
	class error: public std::exception {
	private:
		std::string _message;
		size_t _line_number;
		size_t _char_index;
	public:
		error(std::string message, size_t line_number, size_t char_index) noexcept;
		
		const char* what() const noexcept override;
		size_t line_number() const noexcept;
		size_t char_index() const noexcept;
	};
	
	error parsing_error(std::string_view message, size_t line_number, size_t char_index);
	error syntax_error(std::string_view message, size_t line_number, size_t char_index);
	error semantic_error(std::string_view message, size_t line_number, size_t char_index);
	error compiler_error(std::string_view message, size_t line_number, size_t char_index);

	error unexpected_error(std::string_view unexpected, size_t line_number, size_t char_index);
	error unexpected_syntax_error(std::string_view unexpected, size_t line_number, size_t char_index);
	error undeclared_error(std::string_view undeclared, size_t line_number, size_t char_index);
	error wrong_type_error(std::string_view source, std::string_view destination, bool lvalue,
	                       size_t line_number, size_t char_index);

	using get_character = std::function<int()>;
	void format_error(const error& err, get_character source, std::ostream& output);
	
	
	class runtime_error: public std::exception {
	private:
		std::string _message;
	public:
		runtime_error(std::string message) noexcept;
		
		const char* what() const noexcept override;
	};
	
	void runtime_assertion(bool b, const char* message);
};

#endif /* errors_hpp */
