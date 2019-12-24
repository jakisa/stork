#include <cstdio>
#include "tokenizer.hpp"
#include <iostream>
#include "errors.hpp"

int main() {
	using namespace stork;
	
	if(FILE* fp = fopen("test.stk", "rb")) {
		get_character input = [fp]() {
			return fgetc(fp);
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
		} catch(const error& err) {
			fseek(fp, 0, SEEK_SET);
			format_error(err, input, std::cerr);
		}
		
		fclose(fp);
	}
	return 0;
}
