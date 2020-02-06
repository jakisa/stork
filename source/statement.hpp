#ifndef statement_hpp
#define statement_hpp
#include <memory>

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
	
	using shared_statement_ptr=std::shared_ptr<statement>;
}


#endif /* statement_hpp */
