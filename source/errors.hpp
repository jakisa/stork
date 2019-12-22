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
	
	void throw_parsing_error(const char* message, size_t line_number, size_t char_index);
	void throw_unexpected_error(std::string_view unexpected, size_t line_number, size_t char_index);

	using get_character = std::function<int()>;
	void format_error(const error& err, get_character source, std::ostream& output);
};

#endif /* errors_hpp */
