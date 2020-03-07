#ifndef expression_tree_parser_hpp
#define expression_tree_parser_hpp

#include <memory>

#include "types.hpp"

namespace stork {
	struct node;
	using node_ptr=std::unique_ptr<node>;

	class tokens_iterator;

	class compiler_context;

	node_ptr parse_expression_tree(compiler_context& context, tokens_iterator& it, type_handle type_id, bool allow_comma);
}

#endif /* expression_tree_parser_hpp */
