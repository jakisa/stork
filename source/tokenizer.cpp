#include "tokenizer.hpp"
#include <map>
#include <string>
#include <cctype>
#include <stack>
#include <cstdlib>
#include "push_back_stream.hpp"
#include "errors.hpp"

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
		
		void fetch_word(
			push_back_stream& stream,
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
			
			stream.push_back(c);
			
			if (std::optional<reserved_token> token  = get_keyword(word)) {
				on_reserved_token(*token);
			} else {
				if (std::isdigit(word.front())) {
					char* endptr;
					double num = strtod(word.c_str(), &endptr);
					if (*endptr != 0) {
						num = strtol(word.c_str(), &endptr, 0);
						if (*endptr != 0) {
							throw_unexpected_error(std::string(1, char(*endptr)), stream.line_number(), stream.char_index());
						}
					}
					on_number(num);
				} else {
					on_identifier(word);
				}
			}
		}
		
		void fetch_operator(push_back_stream& stream, const reserved_token_callback& on_reserved_token) {
			if (std::optional<reserved_token> token = get_operator(stream)) {
				on_reserved_token(*token);
			} else {
				std::string unexpected;
				size_t line_number = stream.line_number();
				size_t char_index = stream.char_index();
				for (int c = stream(); get_character_type(c) == character_type::punct; c = stream()) {
					unexpected.push_back(char(c));
				}
				throw_unexpected_error(unexpected, line_number, char_index);
			}
		}
		
		void fetch_string(push_back_stream& stream, const string_callback& on_string) {
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
								throw_parsing_error("Expected closing '\"'", stream.line_number(), stream.char_index());
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
			
			throw_parsing_error("Expected closing '\"'", stream.line_number(), stream.char_index());
		}
		
		void skip_line_comment(push_back_stream& stream) {
			int c;
			do {
				c = stream();
			} while (c != '\n' && get_character_type(c) != character_type::eof);
			
			if (c != '\n') {
				stream.push_back(c);
			}
		}
		
		void skip_block_comment(push_back_stream& stream) {
			bool closing = false;
			int c;
			do {
				c = stream();
				if (closing && c == '/') {
					return;
				}
				closing = (c == '*');
			} while (get_character_type(c) != character_type::eof);

			throw_parsing_error("Expected closing '*/'", stream.line_number(), stream.char_index());
			stream.push_back(c);
		}
	}
	
	void tokenize(
		const get_character& input,
		const reserved_token_callback& on_reserved_token,
		const identifier_callback& on_identifier,
		const number_callback& on_number,
		const string_callback& on_string
	) {
		push_back_stream stream(input);
		while (true) {
			int c = stream();
			switch (get_character_type(c)) {
				case character_type::eof:
					return;
				case character_type::space:
					continue;
				case character_type::alphanum:
					stream.push_back(c);
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
									stream.push_back(c1);
							}
						}
						default:
							stream.push_back(c);
							fetch_operator(stream, on_reserved_token);
					}
					break;
			}
		}
	}
}

