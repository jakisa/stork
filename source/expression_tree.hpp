#ifndef expression_tree_hpp
#define expression_tree_hpp
#include <variant>
#include <vector>
#include "tokens.hpp"
#include "types.hpp"
#include "compiler_context.hpp"

namespace stork {
	enum struct node_operation {
		preinc,
		predec,
		postinc,
		postdec,
		positive,
		negative,
		bnot,
		lnot,
		
		add,
		sub,
		mul,
		div,
		idiv,
		mod,
		band,
		bor,
		bxor,
		bsl,
		bsr,
		concat,
		assign,
		add_assign,
		sub_assign,
		mul_assign,
		div_assign,
		idiv_assign,
		mod_assign,
		band_assign,
		bor_assign,
		bxor_assign,
		bsl_assign,
		bsr_assign,
		concat_assign,
		eq,
		ne,
		lt,
		gt,
		le,
		ge,
		comma,
		land,
		lor,
		index,
		
		ternary,
		
		call,
	};
	
	struct node;
	using node_ptr=std::unique_ptr<node>;
	
	struct node {
	private:
		using node_value=std::variant<node_operation, std::string, double, identifier>;
		node_value _value;
		std::vector<node_ptr> _children;
		type_handle _type_id;
		bool _lvalue;
		type_handle _return_type_id;
		bool _returns_lvalue;
		size_t _line_number;
		size_t _char_index;
		
		type_handle get_type_id() const;
		bool is_lvalue() const;
	public:
		node(compiler_context& context, node_value value, std::vector<node_ptr> children, size_t line_number, size_t char_index);
		
		bool is_node_operation() const;
		bool is_identifier() const;
		bool is_number() const;
		bool is_string() const;
		
		node_operation get_node_operation() const;
		std::string_view get_identifier() const;
		double get_number() const;
		std::string_view get_string() const;

		const std::vector<node_ptr>& children() const;
		
		type_handle get_return_type_id() const;
		bool returns_lvalue() const;
		
		size_t line_number() const;
		size_t char_index() const;
		
		void set_return_type(type_handle return_type_id, bool returns_lvalue);
	};

}

#endif /* expression_tree_hpp */
