#include <unordered_map>
#include "statement.hpp"
#include "expression.hpp"
#include "runtime_context.hpp"

namespace stork {
	flow::flow(flow_type type, int break_level):
		_type(type),
		_break_level(break_level)
	{
	}

	flow_type flow::type() const {
		return _type;
	}
	
	int flow::break_level() const {
		return _break_level;
	}
	
	flow flow::normal_flow() {
		return flow(flow_type::f_normal, 0);
	}

	flow flow::break_flow(int break_level) {
		return flow(flow_type::f_break, break_level);
	}
	
	flow flow::continue_flow() {
		return flow(flow_type::f_continue, 0);
	}
	
	flow flow::return_flow() {
		return flow(flow_type::f_return, 0);
	}
	
	flow flow::consume_break() {
		return _break_level == 1 ? flow::normal_flow() : flow::break_flow(_break_level-1);
	}
	
	namespace {
		class simple_statement: public statement {
		private:
			expression<void>::ptr _expr;
		public:
			simple_statement(expression<void>::ptr expr):
				_expr(std::move(expr))
			{
			}
			
			flow execute(runtime_context& context) override {
				_expr->evaluate(context);
				return flow::normal_flow();
			}
		};
		
		class block_statement: public statement {
		private:
			std::vector<statement_ptr> _statements;
		public:
			block_statement(std::vector<statement_ptr> statements):
				_statements(std::move(statements))
			{
			}
			
			flow execute(runtime_context& context) override {
				auto _ = context.enter_scope();
				for (const statement_ptr& statement : _statements) {
					if (flow f = statement->execute(context); f.type() != flow_type::f_normal) {
						return f;
					}
				}
				return flow::normal_flow();
			}
		};
			
		class local_declaration_statement: public statement {
		private:
			std::vector<expression<lvalue>::ptr> _decls;
		public:
			local_declaration_statement(std::vector<expression<lvalue>::ptr> decls):
				_decls(std::move(decls))
			{
			}
			
			flow execute(runtime_context& context) override {
				for (const expression<lvalue>::ptr& decl : _decls) {
					context.push(decl->evaluate(context));
				}
				return flow::normal_flow();
			}
		};
			
		class break_statement: public statement {
		private:
			int _break_level;
		public:
			break_statement(int break_level):
				_break_level(break_level)
			{
			}
			
			flow execute(runtime_context&) override {
				return flow::break_flow(_break_level);
			}
		};
		
		class continue_statement: public statement {
		public:
			continue_statement() = default;
			
			flow execute(runtime_context&) override {
				return flow::continue_flow();
			}
		};
		
		class return_statement: public statement {
		private:
			expression<lvalue>::ptr _expr;
		public:
			return_statement(expression<lvalue>::ptr expr) :
				_expr(std::move(expr))
			{
			}
			
			flow execute(runtime_context& context) override {
				context.retval() = _expr->evaluate(context);
				return flow::return_flow();
			}
		};
		
		class return_void_statement: public statement {
		public:
			return_void_statement() = default;
			
			flow execute(runtime_context&) override {
				return flow::return_flow();
			}
		};
		
		class if_statement: public statement {
		private:
			std::vector<expression<number>::ptr> _exprs;
			std::vector<statement_ptr> _statements;
		public:
			if_statement(std::vector<expression<number>::ptr> exprs, std::vector<statement_ptr> statements):
				_exprs(std::move(exprs)),
				_statements(std::move(statements))
			{
			}
			
			flow execute(runtime_context& context) override {
				for (size_t i = 0; i < _exprs.size(); ++i) {
					if (_exprs[i]->evaluate(context)) {
						return _statements[i]->execute(context);
					}
				}
				return _statements.back()->execute(context);
			}
		};
		
		class if_declare_statement: public if_statement {
		private:
			std::vector<expression<lvalue>::ptr> _decls;
		public:
			if_declare_statement(
				std::vector<expression<lvalue>::ptr> decls,
				std::vector<expression<number>::ptr> exprs,
				std::vector<statement_ptr> statements
			):
				if_statement(std::move(exprs), std::move(statements)),
				_decls(std::move(decls))
			{
			}
			
			flow execute(runtime_context& context) override {
				auto _ = context.enter_scope();
				
				for (const expression<lvalue>::ptr& decl : _decls) {
					context.push(decl->evaluate(context));
				}
				
				return if_statement::execute(context);
			}
		};
		
		class switch_statement: public statement {
		private:
			expression<number>::ptr _expr;
			std::vector<statement_ptr> _statements;
			std::unordered_map<number, size_t> _cases;
			size_t _dflt;
		public:
			switch_statement(
				expression<number>::ptr expr,
				std::vector<statement_ptr> statements,
				std::unordered_map<number, size_t> cases,
				size_t dflt
			):
				_expr(std::move(expr)),
				_statements(std::move(statements)),
				_cases(std::move(cases)),
				_dflt(dflt)
			{
			}
			
			flow execute(runtime_context& context) override {
				auto it = _cases.find(_expr->evaluate(context));
				for (size_t idx = (it == _cases.end() ? _dflt : it->second); idx < _statements.size(); ++idx) {
					switch (flow f = _statements[idx]->execute(context); f.type()) {
						case flow_type::f_normal:
							break;
						case flow_type::f_break:
							return f.consume_break();
						default:
							return f;
					}
				}
				
				return flow::normal_flow();
			}
		};
		
		class switch_declare_statement: public switch_statement {
		private:
			std::vector<expression<lvalue>::ptr> _decls;
		public:
			switch_declare_statement(
				std::vector<expression<lvalue>::ptr> decls,
				expression<number>::ptr expr,
				std::vector<statement_ptr> statements,
				std::unordered_map<number, size_t> cases,
				size_t dflt
			):
				_decls(std::move(decls)),
				switch_statement(std::move(expr), std::move(statements), std::move(cases), dflt)
			{
			}
			
			flow execute(runtime_context& context) override {
				auto _ = context.enter_scope();
			
				for (const expression<lvalue>::ptr& decl : _decls) {
					context.push(decl->evaluate(context));
				}
				
				return switch_statement::execute(context);
			}
		};
		
		class while_statement: public statement {
		private:
			expression<number>::ptr _expr;
			statement_ptr _statement;
		public:
			while_statement(expression<number>::ptr expr, statement_ptr statement):
				_expr(std::move(expr)),
				_statement(std::move(statement))
			{
			}
			
			flow execute(runtime_context& context) override {
				while (_expr->evaluate(context)) {
					switch (flow f = _statement->execute(context); f.type()) {
						case flow_type::f_normal:
						case flow_type::f_continue:
							break;
						case flow_type::f_break:
							return f.consume_break();
						case flow_type::f_return:
							return f;
					}
				}
				
				return flow::normal_flow();
			}
		};
		
		class do_statement: public statement {
		private:
			expression<number>::ptr _expr;
			statement_ptr _statement;
		public:
			do_statement(expression<number>::ptr expr, statement_ptr statement):
				_expr(std::move(expr)),
				_statement(std::move(statement))
			{
			}
			
			flow execute(runtime_context& context) override {
				do {
					switch (flow f = _statement->execute(context); f.type()) {
						case flow_type::f_normal:
						case flow_type::f_continue:
							break;
						case flow_type::f_break:
							return f.consume_break();
						case flow_type::f_return:
							return f;
					}
				} while (_expr->evaluate(context));
				
				return flow::normal_flow();
			}
		};
		
		class for_statement_base: public statement {
		private:
			expression<number>::ptr _expr2;
			expression<void>::ptr _expr3;
			statement_ptr _statement;
		public:
			for_statement_base(
				expression<number>::ptr expr2,
				expression<void>::ptr expr3,
				statement_ptr statement
			):
				_expr2(std::move(expr2)),
				_expr3(std::move(expr3)),
				_statement(std::move(statement))
			{
			}
			
			flow execute(runtime_context& context) override {
				for (; _expr2->evaluate(context); _expr3->evaluate(context)) {
					switch (flow f = _statement->execute(context); f.type()) {
						case flow_type::f_normal:
						case flow_type::f_continue:
							break;
						case flow_type::f_break:
							return f.consume_break();
						case flow_type::f_return:
							return f;
					}
				}
				
				return flow::normal_flow();
			}
		};
		
		class for_statement: public for_statement_base {
		private:
			expression<void>::ptr _expr1;
		public:
			for_statement(
				expression<void>::ptr expr1,
				expression<number>::ptr expr2,
				expression<void>::ptr expr3,
				statement_ptr statement
			):
				for_statement_base(std::move(expr2), std::move(expr3), std::move(statement)),
				_expr1(std::move(expr1))
			{
			}
			
			flow execute(runtime_context& context) override {
				_expr1->evaluate(context);
				
				return for_statement_base::execute(context);
			}
		};
		
		class for_declare_statement: public for_statement_base {
		private:
			std::vector<expression<lvalue>::ptr> _decls;
			expression<number>::ptr _expr2;
			expression<void>::ptr _expr3;
			statement_ptr _statement;
		public:
			for_declare_statement(
				std::vector<expression<lvalue>::ptr> decls,
				expression<number>::ptr expr2,
				expression<void>::ptr expr3,
				statement_ptr statement
			):
				for_statement_base(std::move(expr2), std::move(expr3), std::move(statement)),
				_decls(std::move(decls))
			{
			}
			
			flow execute(runtime_context& context) override {
				auto _ = context.enter_scope();
				
				for (const expression<lvalue>::ptr& decl : _decls) {
					context.push(decl->evaluate(context));
				}

				return for_statement_base::execute(context);
			}
		};
	}
	
	statement_ptr create_simple_statement(expression<void>::ptr expr) {
		return std::make_unique<simple_statement>(std::move(expr));
	}
	
	statement_ptr create_local_declaration_statement(std::vector<expression<lvalue>::ptr> decls) {
		return std::make_unique<local_declaration_statement>(std::move(decls));
	}
	
	statement_ptr create_block_statement(std::vector<statement_ptr> statements) {
		return std::make_unique<block_statement>(std::move(statements));
	}
	
	shared_statement_ptr create_shared_block_statement(std::vector<statement_ptr> statements) {
		return std::make_shared<block_statement>(std::move(statements));
	}

	statement_ptr create_break_statement(int break_level) {
		return std::make_unique<break_statement>(break_level);
	}
	
	statement_ptr create_continue_statement() {
		return std::make_unique<continue_statement>();
	}
	
	statement_ptr create_return_statement(expression<lvalue>::ptr expr) {
		return std::make_unique<return_statement>(std::move(expr));
	}
	
	statement_ptr create_return_void_statement() {
		return std::make_unique<return_void_statement>();
	}
	
	statement_ptr create_if_statement(
		std::vector<expression<lvalue>::ptr> decls,
		std::vector<expression<number>::ptr> exprs,
		std::vector<statement_ptr> statements
	) {
		if (!decls.empty()) {
			return std::make_unique<if_declare_statement>(std::move(decls), std::move(exprs), std::move(statements));
		} else {
			return std::make_unique<if_statement>(std::move(exprs), std::move(statements));
		}
	}
	
	statement_ptr create_switch_statement(
		std::vector<expression<lvalue>::ptr> decls,
		expression<number>::ptr expr,
		std::vector<statement_ptr> statements,
		std::unordered_map<number, size_t> cases,
		size_t dflt
	) {
		if (!decls.empty()) {
			return std::make_unique<switch_declare_statement>(
				std::move(decls),
				std::move(expr),
				std::move(statements),
				std::move(cases), dflt
			);
		} else {
			return std::make_unique<switch_statement>(
				std::move(expr),
				std::move(statements),
				std::move(cases), dflt
			);
		}
	}
	
	
	statement_ptr create_while_statement(expression<number>::ptr expr, statement_ptr statement) {
		return std::make_unique<while_statement>(std::move(expr), std::move(statement));
	}
	
	statement_ptr create_do_statement(expression<number>::ptr expr, statement_ptr statement) {
		return std::make_unique<do_statement>(std::move(expr), std::move(statement));
	}
	
	statement_ptr create_for_statement(
		expression<void>::ptr expr1,
		expression<number>::ptr expr2,
		expression<void>::ptr expr3,
		statement_ptr statement
	) {
		return std::make_unique<for_statement>(std::move(expr1), std::move(expr2), std::move(expr3), std::move(statement));
	}
	
	statement_ptr create_for_statement(
		std::vector<expression<lvalue>::ptr> decls,
		expression<number>::ptr expr2,
		expression<void>::ptr expr3,
		statement_ptr statement
	) {
		return std::make_unique<for_declare_statement>(std::move(decls), std::move(expr2), std::move(expr3), std::move(statement));
	}
}
