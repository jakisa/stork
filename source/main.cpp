#include <iostream>
#include <sstream>
#include "tokenizer.hpp"
#include "errors.hpp"
#include "compiler_context.hpp"
#include "expression.hpp"
#include "runtime_context.hpp"
#include "push_back_stream.hpp"
#include "compiler.hpp"

// When debugging in Xcode, setting the breakpoint will send eof to stdin. We need to ignore it.
#define XCODE_DEBUG_HACK

const char* stork_code = R"STORK_CODE(

var number global_x(100);

public function void main(){
	global_x = 0;
	for (var var number i(1); i <= 10; ++i) {
		global_x += i;
	}
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
	
		rctx.call_public_function("main");
	
		variable_ptr ret = rctx.end_function(0);
		
		std::cout << rctx.global(0)->to_string() << std::endl;
	} catch (const error& err) {
		code = stork_code;
		format_error(err, get_character, std::cerr);
	}
	
	return 0;
}
