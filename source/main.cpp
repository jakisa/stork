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
			tokenize(
				input,
				[](reserved_token t) {
					std::cout << "Token: " << t << std::endl;
				},
				[](std::string_view id) {
					std::cout << "Identifier: " << id << std::endl;
				},
				[](double x) {
					std::cout << "Number: " << x << std::endl;
				},
				[](std::string_view s) {
					std::cout << "String: " << s << std::endl;
				}
			);
		} catch(const error& err) {
			fseek(fp, 0, SEEK_SET);
			format_error(err, input, std::cerr);
		}
		
		fclose(fp);
	}
	return 0;
}
