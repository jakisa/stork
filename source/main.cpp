#include <iostream>
#include <sstream>
#include "tokenizer.hpp"
#include "errors.hpp"

// When debugging in Xcode, setting the breakpoint will send eof to stdin. We need to ignore it.
// #define XCODE_DEBUG_HACK

int main() {
	using namespace stork;
	std::cerr << "Enter the line to tokenize, or newline to exit." << std::endl;
	std::string line;
	
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
				for (token t = tokenize(stream); !t.is_eof(); t = tokenize(stream)) {
					if (t.is_reserved_token()) {
						std::cout << "Reserved: " << t.get_reserved_token() << std::endl;
					} else if (t.is_identifier()) {
						std::cout << "Identifier: " << t.get_identifier() << std::endl;
					} else if (t.is_number()) {
						std::cout << "Number: " << t.get_number() << std::endl;
					} else if (t.is_string()) {
						std::cout << "String: " << t.get_string() << std::endl;
					}
				}
			} catch (const error& err) {
				strstream.clear();
				strstream.seekg(0);
				format_error(err, input, std::cerr);
			}
		} else {
		}
	}while(
		!line.empty()
#ifdef XCODE_DEBUG_HACK
		|| std::cin.eof()
#endif
	);
	return 0;
}
