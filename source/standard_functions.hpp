#ifndef standard_functions_hpp
#define standard_functions_hpp

namespace stork {

	class stork_module;
	
	void add_math_functions(stork_module& m);
	void add_string_functions(stork_module& m);
	void add_trace_functions(stork_module& m);
	
	void add_standard_functions(stork_module& m);
}

#endif /* standard_functions_hpp */
