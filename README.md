# Stork
A small footprint scripting language with C++ integration.

In this branch, expression parser is implemented. You can compile this code and then execute it.
You can type-in expressions and each expression will be echo-ed on the output, but will added brackets, to proof that the expression is parsed correctly.

You can assume that there are 3 number variables (a, b, c), 3 number constants (d, e, f), 3 string variables (str1, str2, str3), 3 string constants (str4, str5, str6), function "add", that receives two numbers and returns a number, function "concat_to" that receives one string by reference, one string by value and returns the string. There are also two arrays: numarr, that holds numbers, and strarr, that holds strings. 

This project is a work in progress. It is tested under Xcode on Mac. You should be able to compile it with C++17 standard on any platform.
