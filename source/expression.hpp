#ifndef expression_hpp
#define expression_hpp

#include "runtime_context.hpp"

#include <string>

namespace stork {
	template <class T>
	class expression {
		expression(const expression&) = delete;
		void operator=(const expression&) = delete;
	protected:
		expression() = default;
	public:
		using ptr = std::unique_ptr<const expression>;
		
		virtual T evaluate(runtime_context& context) const = 0;
		virtual ~expression() = default;
	};
	
	using void_expression = expression<void>;
	using number_expression = expression<number>;
}

#endif /* expression_hpp */
