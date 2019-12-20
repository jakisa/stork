#ifndef errors_hpp
#define errors_hpp

#include <exception>
#include <string>
#include <string_view>

namespace lightscript {
	class error: public std::exception {
	private:
		std::string _message;
		size_t _line_number;
	public:
		error(std::string message, size_t line_number) noexcept;
		
		const char* what() const noexcept override;
		size_t line_number() const noexcept;
	};
	
	void throw_parsing_error(const char* message, size_t line_number);
	void throw_unexpected_error(std::string_view unexpected, size_t line_number);
};

#endif /* errors_hpp */
