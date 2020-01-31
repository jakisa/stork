#include "statement.hpp"
#include "expression.hpp"
#include <unordered_map>

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
	
	class empty_statement: public statement {
	public:
		empty_statement() = default;
		
		flow execute(runtime_context&) override {
			return flow::normal_flow();
		}
	};
	
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
			for (const statement_ptr& statement : _statements) {
				switch (flow f = statement->execute(context); f.type()) {
					case flow_type::f_normal:
						break;
					default:
						return f;
				}
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
	
	class for_statement: public statement {
	private:
		expression<void>::ptr _expr1;
		expression<number>::ptr _expr2;
		expression<void>::ptr _expr3;
		statement_ptr _statement;
	public:
		for_statement(
			expression<void>::ptr expr1,
			expression<number>::ptr expr2,
			expression<void>::ptr expr3,
			statement_ptr statement
		):
			_expr1(std::move(expr1)),
			_expr2(std::move(expr2)),
			_expr3(std::move(expr3)),
			_statement(std::move(statement))
		{
		}
		
		flow execute(runtime_context& context) override {
			for (_expr1->evaluate(context); _expr2->evaluate(context); _expr3->evaluate(context)) {
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
}
