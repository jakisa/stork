#include <cstdio>
#include "tokenizer.hpp"
#include <iostream>
#include "errors.hpp"

int main() {
	using namespace lightscript;
	
	if(FILE* fp = fopen("test.lts", "rb")) {
		try {
			tokenize(
				[fp](){
					return fgetc(fp);
				},
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
			std::cerr << err.what() << std::endl;
		}
		
		fclose(fp);
	}
	return 0;
}
