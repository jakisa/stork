#include <iostream>
#include <sstream>
#include "tokenizer.hpp"
#include "errors.hpp"
#include "compiler_context.hpp"
#include "expression.hpp"
#include "runtime_context.hpp"
#include "push_back_stream.hpp"
#include "compiler.hpp"
#include <stdlib.h>

const char* stork_code1 = R"STORK_CODE(

public function number fib_recursive(number idx) {
	return idx <= 1 ? idx : fib_recursive(idx-2) + fib_recursive(idx-1);
}

public function number fib(number idx) {
	if (idx == 0) {
		return 0;
	}
	
	number fib0 = 0, fib1 = 1;
	
	for (number i = 1; i < idx; ++i) {
		number fib2 = fib0 + fib1;
		fib0 = fib1;
		fib1 = fib2;
	}
	
	return fib1;
}

function number initJ() {
	return 10;
}

number I = 10;
number J = initJ();

public function number test_size() {
	number[] var;
	
	for (number i = 0; i < 10; ++i) {
		for (number j = 0; j < 10; ++j) {
			var[sizeof(var)] = 1;
		}
	}

	return sizeof(var);
}

function void swap(number& x, number& y) {
	number tmp = x;
	x = y;
	y = tmp;
}

function number less(number x, number y) {
	return x < y;
}

function number greater(number x, number y) {
	return x > y;
}

function void quicksort(number[]& arr, number begin, number end, number(number, number) less) {
	if (end - begin < 2)
		return;
	
	number pivot = arr[end-1];

	number i = begin;
	
	for (number j = begin; j < end-1; ++j)
		if (less(arr[j], pivot))
			swap(&arr[i++], &arr[j]);
	
	swap (&arr[i], &arr[end-1]);

	quicksort(&arr, begin, i, less);
	quicksort(&arr, i+1, end, less);
}

public function void sort(number[]& arr) {
	quicksort(&arr, 0, sizeof(arr), greater);
}

public function number[] test() {
	number[] arr = {3, 5, 1, 8, 3, 7};
	return arr;
	/*<number, string>[] tuples;
	tuples[0][0] = 1;
	tuples[0][1] = "abc";
	tuples[1][0] = 2;
	tuples[1][1] = "def";
	tuples[2] = {3, "ghi"};
	
	number[] arr = {6, 2, tuples[2][0], 3, 4, 5, 6, 7, 1};
	
	sort(&arr);
	
	//<number, string> tuple = {1, "aaa"};
	
	return arr;*/
}

)STORK_CODE";

const char* stork_code = R"STORK_CODE(

function number[] get_arr() {
	return {1, 2, 3, 4, 5};
}

public function string test() {
	//number n = get_arr()[3];
	
	//return n;
	return 42;
}

)STORK_CODE";

int main() {
	using namespace stork;
	
	compiler_context ctx;
	
	const char* code = stork_code;
	
	std::function<int()> get_character = [&code] () mutable {
		if (*code) {
			return int(*(code++));
		} else {
			return -1;
		}
	};
	
	push_back_stream stream(&get_character);
	
	tokens_iterator it(stream);
	
	try {
		runtime_context rctx = compile(ctx, it);
/*
		std::cout <<
			rctx.call(
				rctx.get_public_function("fib"),
				{std::make_shared<variable_impl<number> >(20)}
			)->to_string()
		<< std::endl;
		
		std::cout <<
			rctx.call(
				rctx.get_public_function("fib_recursive"),
				{std::make_shared<variable_impl<number> >(20)}
			)->to_string()
		<< std::endl;

		std::cout <<
			rctx.call(
				rctx.get_public_function("test_size"),
				{}
			)->to_string()
		<< std::endl;
*/
		std::cout <<
			rctx.call(
				rctx.get_public_function("test"),
				{}
			)->to_string()
		<< std::endl;
/*
		larray arr = std::make_shared<variable_impl<array> >(array());
		srand((unsigned int)time(0));
		for (int i = 0; i < 1000; ++i) {
			arr->value.push_back(std::make_shared<variable_impl<number> >(rand() % 100));
		}
		
		rctx.call(
			rctx.get_public_function("sort"),
			{arr}
		);
		
		std::cout << std::is_sorted(arr->value.begin(), arr->value.end(), [](variable_ptr l, variable_ptr r){
			return l->static_pointer_downcast<lnumber>()->value > r->static_pointer_downcast<lnumber>()->value;
		})  << std::endl;
*/
	} catch (const error& err) {
		code = stork_code;
		format_error(err, get_character, std::cerr);
	} catch (const runtime_error& err) {
		std::cerr << err.what() << std::endl;
	}
	
	return 0;
}
