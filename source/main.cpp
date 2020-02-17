#include <iostream>
#include "module.hpp"

int main() {
	std::string path = __FILE__;
	path = path.substr(0, path.find_last_of("/\\") + 1) + "test.stk";
	
	std::cout << path << std::endl;

	using namespace stork;
	
	module m;
	
	m.add_external_function("greater", std::function<number(number, number)>([](number x, number y){
		return x > y;
	}));
	
	auto test = m.create_public_function_caller<number>("test");
	
	if (m.try_load(path.c_str(), &std::cerr)) {
		std::cout << test() << std::endl;
	}
	
	return 0;
}
