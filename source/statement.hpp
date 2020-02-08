#ifndef statement_hpp
#define statement_hpp
#include <memory>
#include <vector>
#include "expression.hpp"

namespace stork {
	enum struct flow_type{
		f_normal,
		f_break,
		f_continue,
		f_return,
	};
	
	class flow {
	private:
		flow_type _type;
		int _break_level;
		flow(flow_type type, int break_level);
	public:
		flow_type type() const;
		int break_level() const;
		
		static flow normal_flow();
		static flow break_flow(int break_level);
		static flow continue_flow();
		static flow return_flow();
		flow consume_break();
	};
	
	class runtime_context;
	
	class statement {
		statement(const statement&) = delete;
		void operator=(const statement&) = delete;
	protected:
		statement() = default;
	public:
		virtual flow execute(runtime_context& context) = 0;
		virtual ~statement() = default;
	};
	
	using statement_ptr=std::unique_ptr<statement>;
	
	class block_statement: public statement {
	private:
		std::vector<statement_ptr> _statements;
		size_t _scope_vars;
	public:
		block_statement(std::vector<statement_ptr> statements, size_t scope_vars):
			_statements(std::move(statements)),
			_scope_vars(scope_vars)
		{
		}
		
		flow execute(runtime_context& context) override {
			for (const statement_ptr& statement : _statements) {
				switch (flow f = statement->execute(context); f.type()) {
					case flow_type::f_normal:
						break;
					default:
						return f;
				}
			}
			context.end_scope(_scope_vars);
			return flow::normal_flow();
		}
	};
	
	using block_statement_ptr = std::unique_ptr<block_statement>;
	using shared_block_statement_ptr = std::shared_ptr<block_statement>;
	
	statement_ptr create_empty_statement();
	
	statement_ptr create_simple_statement(expression<void>::ptr expr);
	
	statement_ptr create_global_declaration_statement(int idx, expression<retval>::ptr expr);
	
	statement_ptr create_local_declaration_statement(expression<retval>::ptr expr);
	
	block_statement_ptr create_block_statement(std::vector<statement_ptr> statements, size_t scope_vars);
	shared_block_statement_ptr create_shared_block_statement(std::vector<statement_ptr> statements, size_t scope_vars);
	
	statement_ptr create_break_statement(int break_level);
	
	statement_ptr create_continue_statement();
	
	statement_ptr create_return_statement(expression<retval>::ptr expr);
	
	statement_ptr create_return_void_statement();
	
	statement_ptr create_if_statement(std::vector<expression<number>::ptr> exprs, std::vector<block_statement_ptr> statements);
	
	statement_ptr create_if_declare_statement(
		expression<retval>::ptr declexpr,
		std::vector<expression<number>::ptr> exprs,
		std::vector<block_statement_ptr> statements
	);
	
	statement_ptr create_switch_statement(
		expression<number>::ptr expr,
		std::vector<statement_ptr> statements,
		std::unordered_map<number, size_t> cases,
		size_t dflt
	);

	statement_ptr create_switch_declare_statement(
		expression<retval>::ptr declexpr,
		expression<number>::ptr expr,
		std::vector<statement_ptr> statements,
		std::unordered_map<number, size_t> cases,
		size_t dflt
	);
	
	
	statement_ptr crete_while_statement(expression<number>::ptr expr, block_statement_ptr statement);
	
	statement_ptr create_do_statement(expression<number>::ptr expr, block_statement_ptr statement);
	
	statement_ptr create_for_statement(
		expression<void>::ptr expr1,
		expression<number>::ptr expr2,
		expression<void>::ptr expr3,
		block_statement_ptr statement
	);
	
	statement_ptr create_for_declare_statement(
		expression<retval>::ptr expr1,
		expression<number>::ptr expr2,
		expression<void>::ptr expr3,
		block_statement_ptr statement
	);
}


#endif /* statement_hpp */
