#include "push_back_stream.hpp"

namespace stork {
	push_back_stream::push_back_stream(const get_character* input) :
		_input(*input),
		_line_number(0),
		_char_index(0)
	{
	}
		
	int push_back_stream::operator()() {
		int ret = -1;
		if (_stack.empty()) {
			ret = _input();
		} else {
			ret = _stack.top();
			_stack.pop();
		}
		if (ret == '\n') {
			++_line_number;
		}
		
		++_char_index;
		
		return ret;
	}
	
	void push_back_stream::push_back(int c) {
		_stack.push(c);
		
		if (c == '\n') {
			--_line_number;
		}
		
		--_char_index;
	}
	
	size_t push_back_stream::line_number() const {
		return _line_number;
	}
	
	size_t push_back_stream::char_index() const {
		return _char_index;
	}
}

