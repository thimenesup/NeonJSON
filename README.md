# NeonJSON

## Features:
* C++17
* Bloat free, small source code (<1k loc) with no external dependencies
* Made to interop natively with the std libc++
* Optional pretty printed output with whitespace and indentation
* Optional compiletime extension for comments
* Faster than certain award-winning multimillion-dollar-profit game parser! (No quadratic parsing by using sscanf)

Has been tested to successfully implement a glTF scene importer

### Example usage:

```C++
#include "JSON.hpp"

int main() {
	
	std::string parseString =
		"{"
			"\"a\": 1337,"
			"\"b\": 3.1416,"
			"\"cool\": true,"
			"\"name\": \"neon\","

			"\"bools\": [ true, false, false, true ],"
			"\"integers\": [ 0, 1, 2, 3, 4 ],"
			"\"floats\": [ 0.2, 2.4, 4.6, 6.7 ],"
			"\"strings\": [ \"one\", \"two\", \"three\" ],"
			"\"mix\" : [ true, 1, 4.6, \"four\" ],"

			"\"dictionary\": {"
				"\"a\": true,"
				"\"b\": 420,"
				"\"c\": 322.322"
		"}";

	std::unordered_map<std::string, Variant> jsonObject = JSON::ParseJSON(parseString);
	
	std::string writeString = JSON::ToJSON(jsonObject, true);

	printf("%s\n", writeString.data());	
	
	return 0;
}
```