#include <iostream>
#include <sstream>
#include "tokenizer.hpp"
#include "errors.hpp"
#include "compiler_context.hpp"
#include "expression.hpp"
#include "runtime_context.hpp"
#include "push_back_stream.hpp"
#include "compiler.hpp"

const char* stork_code = R"STORK_CODE(

public function number fib_recursive(number idx) {
	return idx <= 1 ? idx : fib_recursive(idx-2) + fib_recursive(idx-1);
}

public function number fib(number idx) {
	if (idx == 0) {
		return 0;
	}
	
	number fib0 = 0, fib1 = 1;
	
	for (number i = 1; i < idx; ++i) {
		number fib2 = fib0 + fib1;
		fib0 = fib1;
		fib1 = fib2;
	}
	
	return fib1;
}

public function number test_size() {
	number[] var;
	var[700] = 42;
	return sizeof(var);
}

)STORK_CODE";

int main() {
	using namespace stork;
	
	compiler_context ctx;
	
	const char* code = stork_code;
	
	std::function<int()> get_character = [&code] () mutable {
		if (*code) {
			return int(*(code++));
		} else {
			return -1;
		}
	};
	
	push_back_stream stream(&get_character);
	
	tokens_iterator it(stream);
	
	try {
	
		runtime_context rctx = compile(ctx, it);
		
		std::cout <<
			rctx.call(
				rctx.get_public_function("fib"),
				{std::make_unique<variable_impl<number> >(20)}
			)->to_string()
		<< std::endl;
		
		std::cout <<
			rctx.call(
				rctx.get_public_function("fib_recursive"),
				{std::make_unique<variable_impl<number> >(20)}
			)->to_string()
		<< std::endl;
		
		std::cout <<
			rctx.call(
				rctx.get_public_function("test_size"),
				{}
			)->to_string()
		<< std::endl;
	} catch (const error& err) {
		code = stork_code;
		format_error(err, get_character, std::cerr);
	}
	
	return 0;
}
