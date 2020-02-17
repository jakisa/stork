#ifndef standard_functions_hpp
#define standard_functions_hpp

namespace stork {

	class module;
	
	void add_math_functions(module& m);
	void add_string_functions(module& m);
	void add_trace_functions(module& m);
	
	void add_standard_functions(module& m);
}

#endif /* standard_functions_hpp */
