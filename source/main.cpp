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

public function number fib(number idx) {
	if (idx == 0) {
		return 0;
	}
	
	number[] arr;
	
	arr[0] = 0;
	arr[1] = 1;
	
	for (number i = 2; i <= idx; ++i) {
		arr[i] = arr[i-2] + arr[i-1];
	}
	
	return arr[idx];
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
	
		rctx.push(std::make_unique<variable_impl<number> >(6));
		rctx.call();
		rctx.call_public_function("fib");
		variable_ptr ret = rctx.end_function(0);
		
		std::cout << ret->to_string() << std::endl;
	} catch (const error& err) {
		code = stork_code;
		format_error(err, get_character, std::cerr);
	}
	
	return 0;
}
