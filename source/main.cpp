#include <iostream>
#include <sstream>
#include "tokenizer.hpp"
#include "errors.hpp"
#include "compiler_context.hpp"
#include "expression.hpp"

// When debugging in Xcode, setting the breakpoint will send eof to stdin. We need to ignore it.
// #define XCODE_DEBUG_HACK

namespace {
	using namespace stork;
	
	void create_identifiers(compiler_context& context) {
		context.create_identifier("a", type_registry::get_number_handle(), false);
		context.create_identifier("b", type_registry::get_number_handle(), false);
		context.create_identifier("c", type_registry::get_number_handle(), false);
		context.create_identifier("d", type_registry::get_number_handle(), false);
		context.create_identifier("e", type_registry::get_number_handle(), false);
		context.create_identifier("f", type_registry::get_number_handle(), false);
		
		context.create_identifier("str1", type_registry::get_string_handle(), false);
		context.create_identifier("str2", type_registry::get_string_handle(), false);
		context.create_identifier("str3", type_registry::get_string_handle(), false);
		context.create_identifier("str4", type_registry::get_string_handle(), false);
		context.create_identifier("str5", type_registry::get_string_handle(), false);
		context.create_identifier("str6", type_registry::get_string_handle(), false);
		
		function_type ft1;
		ft1.return_type_id = type_registry::get_number_handle();
		ft1.param_type_id.push_back({type_registry::get_number_handle(), false});
		ft1.param_type_id.push_back({type_registry::get_number_handle(), false});
		
		function_type ft2;
		ft2.return_type_id = type_registry::get_string_handle();
		ft2.param_type_id.push_back({type_registry::get_string_handle(), true});
		ft2.param_type_id.push_back({type_registry::get_string_handle(), false});
		
		context.create_identifier("add", context.get_handle(ft1), true);
		context.create_identifier("concat_to", context.get_handle(ft2), true);
		
		context.create_identifier("numarr", context.get_handle(array_type{type_registry::get_number_handle()}), false);
		context.create_identifier("strarr", context.get_handle(array_type{type_registry::get_string_handle()}), false);
	}
	
	void add(runtime_context& ctx) {
		lnumber a = ctx.local(-1)->static_pointer_downcast<lnumber>();
		lnumber b = ctx.local(-2)->static_pointer_downcast<lnumber>();
		
		ctx.retval() = std::make_shared<variable_impl<number> >(a->value + b->value);
	}
	
	void concat_to(runtime_context& ctx) {
		lstring str1 = ctx.local(-1)->static_pointer_downcast<lstring>();
		lstring str2 = ctx.local(-2)->static_pointer_downcast<lstring>();
		
		str1->value = std::make_shared<std::string>(*str1->value + *str2->value);
		
		ctx.retval() = std::make_shared<variable_impl<string> >(str1->value);
	}
	
	void prepare_runtime_context(runtime_context& context) {
		context.global(0) = std::make_shared<variable_impl<number> >(0);
		context.global(1) = std::make_shared<variable_impl<number> >(0);
		context.global(2) = std::make_shared<variable_impl<number> >(0);
		context.global(3) = std::make_shared<variable_impl<number> >(0);
		context.global(4) = std::make_shared<variable_impl<number> >(0);
		context.global(5) = std::make_shared<variable_impl<number> >(0);
		
		context.global(6) = std::make_shared<variable_impl<string> >(std::make_shared<std::string>(""));
		context.global(7) = std::make_shared<variable_impl<string> >(std::make_shared<std::string>(""));
		context.global(8) = std::make_shared<variable_impl<string> >(std::make_shared<std::string>(""));
		context.global(9) = std::make_shared<variable_impl<string> >(std::make_shared<std::string>(""));
		context.global(10) = std::make_shared<variable_impl<string> >(std::make_shared<std::string>(""));
		context.global(11) = std::make_shared<variable_impl<string> >(std::make_shared<std::string>(""));
	
		context.global(12) = std::make_shared<variable_impl<function> >(&add);
		context.global(13) = std::make_shared<variable_impl<function> >(&concat_to);
		
		array numbers;
		numbers.push_back(std::make_shared<variable_impl<number> >(1));
		numbers.push_back(std::make_shared<variable_impl<number> >(1));
		numbers.push_back(std::make_shared<variable_impl<number> >(2));
		numbers.push_back(std::make_shared<variable_impl<number> >(3));
		numbers.push_back(std::make_shared<variable_impl<number> >(5));
		context.global(14) = std::make_shared<variable_impl<array> >(std::move(numbers));
		
		array strings;
		strings.push_back(std::make_shared<variable_impl<string> >(std::make_shared<std::string>("M")));
		strings.push_back(std::make_shared<variable_impl<string> >(std::make_shared<std::string>("V")));
		strings.push_back(std::make_shared<variable_impl<string> >(std::make_shared<std::string>("E")));
		strings.push_back(std::make_shared<variable_impl<string> >(std::make_shared<std::string>("M")));
		strings.push_back(std::make_shared<variable_impl<string> >(std::make_shared<std::string>("J")));
		context.global(15) = std::make_shared<variable_impl<array> >(std::move(strings));
	}
	
	void trace_runtime_context(runtime_context& context) {
		std::cout << "a = " << context.global(0)->static_pointer_downcast<lnumber>()->value << std::endl;
		std::cout << "b = " << context.global(1)->static_pointer_downcast<lnumber>()->value << std::endl;
		std::cout << "c = " << context.global(2)->static_pointer_downcast<lnumber>()->value << std::endl;
		std::cout << "d = " << context.global(3)->static_pointer_downcast<lnumber>()->value << std::endl;
		std::cout << "e = " << context.global(4)->static_pointer_downcast<lnumber>()->value << std::endl;
		std::cout << "f = " << context.global(5)->static_pointer_downcast<lnumber>()->value << std::endl;
		
		std::cout << "str1 = " << *context.global(6)->static_pointer_downcast<lstring>()->value << std::endl;
		std::cout << "str2 = " << *context.global(7)->static_pointer_downcast<lstring>()->value << std::endl;
		std::cout << "str3 = " << *context.global(8)->static_pointer_downcast<lstring>()->value << std::endl;
		std::cout << "str4 = " << *context.global(9)->static_pointer_downcast<lstring>()->value << std::endl;
		std::cout << "str5 = " << *context.global(10)->static_pointer_downcast<lstring>()->value << std::endl;
		std::cout << "str6 = " << *context.global(11)->static_pointer_downcast<lstring>()->value << std::endl;
		
		std::cout << "numarr = [";
		{
			const char* separator = "";
			for (const variable_ptr& v : context.global(14)->static_pointer_downcast<larray>()->value) {
				std::cout << separator << v->static_pointer_downcast<lnumber>()->value;
				separator = ", ";
			}
			std::cout << "]\n";
		}
		
		std::cout << "strarr = [";
		{
			const char* separator = "";
			for (const variable_ptr& v : context.global(15)->static_pointer_downcast<larray>()->value) {
				std::cout << separator << *(v->static_pointer_downcast<lstring>()->value);
				separator = ", ";
			}
			std::cout << "]\n";
		}
	}
}

int main() {
	using namespace stork;
	std::cerr << "Enter the expression, or newline to exit." << std::endl;
	std::string line;
	
	compiler_context ccontext;
	create_identifiers(ccontext);
	
	runtime_context rcontext(16);
	prepare_runtime_context(rcontext);
	
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
				
				tokens_iterator it(stream);
				
				expression<void>::ptr exp = build_void_expression(ccontext, it);
				
				exp->evaluate(rcontext);
			
				trace_runtime_context(rcontext);
			} catch (const error& err) {
				strstream.clear();
				strstream.seekg(0);
				format_error(err, input, std::cerr);
			} catch (const runtime_error& err) {
				std::cerr << err.what() << std::endl;
			}
		}
	}while(
		!line.empty()
#ifdef XCODE_DEBUG_HACK
		|| std::cin.eof()
#endif
	);
	return 0;
}
