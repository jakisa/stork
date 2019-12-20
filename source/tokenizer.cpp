#include "tokenizer.hpp"
#include <map>
#include <string>
#include <cctype>
#include <stack>
#include <cstdlib>

namespace lightscript {
	namespace {
		
		
		enum struct character_type {
			eof,
			space,
			alphanum,
			punct,
		};
		
		character_type get_character_type(int c) {
			if (c < 0 ) {
				return character_type::eof;
			}
			if (std::isspace(c)) {
				return character_type::space;
			}
			if (std::isalpha(c) || std::isdigit(c) || c == '_') {
				return character_type::alphanum;
			}
			return character_type::punct;
		}
		
		class unreadable_stream {
		private:
			const get_character& _input;
			std::stack<int> _stack;
		public:
			unreadable_stream(const get_character& input) :
				_input(input)
			{
			}
			
			int operator()() {
				if (_stack.empty()) {
					return _input();
				}
				int ret = _stack.top();
				_stack.pop();
				return ret;
			}
			
			void unread(int c) {
				_stack.push(c);
			}
		};
		
		void fetch_word(
			unreadable_stream& stream,
			const reserved_token_callback& on_reserved_token,
			const identifier_callback& on_identifier,
			const number_callback& on_number
		) {
			std::string word;
			
			int c = stream();
			
			bool is_number = isdigit(c);
			
			do {
				word.push_back(char(c));
				c = stream();
			} while (get_character_type(c) == character_type::alphanum || (is_number && c == '.'));
			
			stream.unread(c);
			
			if (std::optional<reserved_token> token  = get_keyword(word)) {
				on_reserved_token(*token);
			} else {
				if (std::isdigit(word.front())) {
					char* endptr;
					double num = strtod(word.c_str(), &endptr);
					if (*endptr != 0) {
						num = strtol(word.c_str(), &endptr, 0);
						if (*endptr != 0) {
							//THROW!!!
						}
					}
					on_number(num);
				} else {
					on_identifier(word);
				}
			}
		}
		
		/* is_token_still_prefix(const std::string& token, const std::stringview& key) {
			return key.size() > token.size() &&
		}*/
		
		enum struct greedy_match_result {
			greedy_match_prefix,
			greedy_match_none,
			greedy_match_same,
		};
		
		//TODO:
		void fetch_operator(unreadable_stream& stream, const reserved_token_callback& on_reserved_token) {
			/*std::string tok;
			
			do {
				tok.push_back(stream());
			} while (*/
		}
		
		void fetch_string(unreadable_stream& stream, const string_callback& on_string) {
			int c = stream();
			
			std::string str;
			
			bool escaped = false;
			do {
				if (c == '\\') {
					escaped = true;
				} else {
					if (escaped) {
						switch(c) {
							case 't':
								str.push_back('\t');
								break;
							case 'n':
								str.push_back('\n');
								break;
							case 'r':
								str.push_back('\r');
								break;
							case '0':
								str.push_back('\0');
								break;
							default:
								str.push_back(c);
								break;
						}
						escaped = false;
					} else {
						switch (c) {
							case '\t':
							case '\n':
							case '\r':
								//THROW!!!
								break;
							case '"':
								on_string(str);
								return;
							default:
								str.push_back(c);
						}
					}
				}
				c = stream();
			} while (get_character_type(c) != character_type::eof);
			
			//THROW!!!
			stream.unread(c);
		}
		
		void skip_line_comment(unreadable_stream& stream) {
			int c;
			do {
				c = stream();
			} while (c != '\n' && get_character_type(c) != character_type::eof);
			
			if (c != '\n') {
				stream.unread(c);
			}
		}
		
		void skip_block_comment(unreadable_stream& stream) {
			bool closing = false;
			int c;
			do {
				c = stream();
				if (closing && c == '/') {
					return;
				}
				closing = (c == '*');
			} while (get_character_type(c) != character_type::eof);

			//THROW!!!
			stream.unread(c);
		}
	}
	
	void tokenize(
		const get_character& input,
		const reserved_token_callback& on_reserved_token,
		const identifier_callback& on_identifier,
		const number_callback& on_number,
		const string_callback& on_string
	) {
		unreadable_stream stream(input);
		while (true) {
			int c = stream();
			switch (get_character_type(c)) {
				case character_type::eof:
					return;
				case character_type::space:
					continue;
				case character_type::alphanum:
					stream.unread(c);
					fetch_word(stream, on_reserved_token, on_identifier, on_number);
					break;
				case character_type::punct:
					switch (c) {
						case '"':
							fetch_string(stream, on_string);
							break;
						case '/':
						{
							char c1 = stream();
							switch(c1) {
								case '/':
									skip_line_comment(stream);
									continue;
								case '*':
									skip_block_comment(stream);
									continue;
								default:
									stream.unread(c1);
							}
						}
						default:
							stream.unread(c);
							fetch_operator(stream, on_reserved_token);
					}
					break;
			}
		}
	}
}

