#ifndef expression_hpp
#define expression_hpp

#include "runtime_context.hpp"
#include "compiler_context.hpp"
#include "tokenizer.hpp"

#include <string>

namespace stork {
	template <typename T>
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
	
	expression<void>::ptr build_void_expression(compiler_context& context, tokens_iterator& it);
	expression<number>::ptr build_number_expression(compiler_context& context, tokens_iterator& it);
}

#endif /* expression_hpp */
