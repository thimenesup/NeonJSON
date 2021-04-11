#include "JSON.hpp"

#include <charconv>

#define PUT_VARIANT(var)\
uint16_t containerType = toContainer.GetType();\
if (containerType == Variant::Dictionary) {\
	std::unordered_map<std::string, Variant>& map = *(std::unordered_map<std::string, Variant>*)toContainer.GetData();\
	map[currentKey] = std::move(var);\
	expectingKey = true;\
}\
else {\
	std::vector<Variant>& vector = *(std::vector<Variant>*)toContainer.GetData();\
	vector.push_back(std::move(var));\
	expectingKey = false;\
}\


std::string JSON::ToJSON(const std::unordered_map<std::string, Variant>& map, bool prettyPrint) {

	std::string string;

	Variant variant = (std::unordered_map<std::string, Variant>*)&map;

	if (prettyPrint)
		WriteValue<true>(variant, string);
	else
		WriteValue<false>(variant, string);

	return std::move(string);
}

std::unordered_map<std::string, Variant> JSON::ParseJSON(const std::string& string) { //TODO?: Additional error checking and strictness (Example: Checking if key is duplicate)

	//Search first for the expected begin, skipping the character on the recursive function avoiding creating an additional dictionary
	size_t i;
	for (i = 0; i < string.size(); ++i) {
		const uint8_t c = string[i];
		if (c == '{') {
			++i; //Skip it
			break;
		}
	}

	std::unordered_map<std::string, Variant> map;

	Variant variant = (std::unordered_map<std::string, Variant>*)&map;
	ParseValue(variant, string, i);

	return std::move(map);
}

template <bool PrettyPrint>
void JSON::WriteValue(const Variant& variant, std::string& string, uint32_t indentation) {

	switch (variant.GetType()) {

	case Variant::Pointer: {
		if (variant.GetData() == nullptr)
			string += "null";

		break;
	}

	case Variant::Bool: {
		bool boolean = bool(variant);
		if (boolean)
			string += "true";
		else
			string += "false";

		break;
	}

	case Variant::Int: {
		int64_t integer = int64_t(variant);
		string += std::to_string(integer);

		break;
	}

	case Variant::Float: {
		double f = double(variant);
		string += std::to_string(f);

		break;
	}

	case Variant::String: {
		std::string str = std::string(variant);

		std::string escapedString = "";
		for (const char c : str) { //Escape quote characters
			if (c == '"')
				escapedString += '\\';
			escapedString += c;
		}

		string += '"' + escapedString + '"';

		break;
	}

	case Variant::VariantArray: {

		if constexpr (PrettyPrint) {
			string += "[\n";

			string.resize(string.size() + indentation);
			string.replace(string.end() - indentation, string.end(), indentation, '\t');
		}
		else {
			string += '[';
		}

		auto& vector = *(std::vector<Variant>*)variant.GetData();
		for (uint32_t i = 0; i < vector.size(); ++i) {
			WriteValue<PrettyPrint>(vector[i], string, indentation + 1);

			if (i < vector.size() - 1) {
				if constexpr (PrettyPrint) {
					string += ",\n";

					string.resize(string.size() + indentation);
					string.replace(string.end() - indentation, string.end(), indentation, '\t');
				}
				else {
					string += ',';
				}

			}
			else if constexpr (PrettyPrint) {
				string += '\n';

				string.resize(string.size() + indentation - 1);
				string.replace(string.end() - (indentation - 1), string.end(), indentation - 1, '\t');
			}

		}

		string += "]";

		break;
	}

	case Variant::Dictionary: {

		if constexpr (PrettyPrint) {
			string += "{\n";

			string.resize(string.size() + indentation);
			string.replace(string.end() - indentation, string.end(), indentation, '\t');
		}
		else {
			string += '{';
		}


		auto& map = *(std::unordered_map<std::string, Variant>*)variant.GetData();
		uint32_t i = 0;
		for (auto& element : map) {

			if constexpr (PrettyPrint) {
				string += '"' + element.first + '"' + " : ";
			}
			else {
				string += '"' + element.first + '"' + ':';
			}

			WriteValue<PrettyPrint>(element.second, string, indentation + 1);

			if (i < map.size() - 1) {

				if constexpr (PrettyPrint) {
					string += ",\n";

					string.resize(string.size() + indentation);
					string.replace(string.end() - indentation, string.end(), indentation, '\t');
				}
				else {
					string += ',';
				}

			}
			else if constexpr (PrettyPrint) {
				string += '\n';

				string.resize(string.size() + indentation - 1);
				string.replace(string.end() - (indentation - 1), string.end(), indentation - 1, '\t');
			}

			++i;
		}

		string += "}";

		break;
	}

	default:
		break;
	}
}


void JSON::ParseValue(Variant& toContainer, const std::string& string, size_t& index) {

	std::string currentKey = "";
	bool expectingKey = toContainer.GetType() == Variant::Dictionary;

	while (true) {
		std::string_view token;
		GetToken(string, index, token);
		if (token.empty())
			break;

		if (token[0] == '"') { //Strings
			std::string escapedString;
			escapedString.reserve(token.size() - 2); //-2 since we ignore quotes

			for (size_t i = 1; i < token.size() - 1; ++i) { //Start at 1 and -1 size to remove quotes
				const uint8_t character = token[i];
				if (character == '\\' && i < token.size() - 2 && token[i + 1] == '"') {
					continue;
				}
				escapedString.push_back(character);
			}

			if (expectingKey) {
				currentKey = escapedString;
				expectingKey = false;
			}
			else {
				Variant variant = escapedString;
				PUT_VARIANT(variant);
			}

			continue;
		}

		if (token[0] == 't') { //if (token == "true") {
			Variant variant = true;
			PUT_VARIANT(variant);
			continue;
		}

		if (token[0] == 'f') { //if (token == "false") {
			Variant variant = false;
			PUT_VARIANT(variant);
			continue;
		}

		if (token[0] == 'n') { //if (token == "null") {
			Variant variant = (void*)nullptr;
			PUT_VARIANT(variant);
			continue;
		}

		if (isdigit(token[0]) || token[0] == '-') { //Numbers: Check for minus '-' too for negative numbers
			bool isInteger = true;
			for (const uint8_t character : token) {
				if (character == '.' || character == 'e') {
					isInteger = false;
					break;
				}
			}

			if (isInteger) {
				int64_t integer = 0;
				std::from_chars(token.data(), token.data() + token.size(), integer);
				Variant variant = integer;
				PUT_VARIANT(variant);
			}
			else {
				double number = 0.0;
				std::from_chars(token.data(), token.data() + token.size(), number);
				Variant variant = number;
				PUT_VARIANT(variant);
			}

			continue;
		}

		if (token[0] == '{') { //if (token == "{") {
			Variant variant = std::unordered_map<std::string, Variant>();
			ParseValue(variant, string, index);
			PUT_VARIANT(variant);
			continue;
		}

		if (token[0] == '[') { //if (token == "[") {
			Variant variant = std::vector<Variant>();
			ParseValue(variant, string, index);
			PUT_VARIANT(variant);
			continue;
		}

		if (token[0] == '}') { //if (token == "}") {
			break;
		}

		if (token[0] == ']') { //if (token == "]") {
			break;
		}
	}

}


void JSON::GetToken(const std::string_view& source, size_t& i, std::string_view& token) {

	for (i; i < source.size(); ++i) {
		const uint8_t character = source[i];

		//Skip whitespace
		if (isspace(character))
			continue;

#ifdef JSON_COMMENT_EXTENSION
		//Skip comments
		if (character == '/' && i + 1 < source.size()) { //Possible comment
			++i;
			const uint8_t& nextCharacter = source[i];
			if (nextCharacter == '/') { //Single line comment
				for (i; i < source.size(); ++i) {
					const uint8_t& endCharacter = source[i];
					if (endCharacter == '\n') { //Comment end
						//++linesParsed;
						break;
					}
				}

				continue;
			}
			else if (nextCharacter == '*') { //Multiline comment
				for (i; i < source.size(); ++i) {
					const uint8_t& endCharacter = source[i];
					if (endCharacter == '\n') {
						//++linesParsed;
					}
					if (endCharacter == '*' && i + 1 < source.size()) {
						++i;
						const uint8_t& nextEndCharacter = source[i];
						if (nextEndCharacter == '/') {
							break;
						}
					}
				}

				continue;
			}
		}
#endif

		if (character == '"') { //String start
			size_t start = i;
			for (i += 1; i < source.size(); ++i) {
				const uint8_t nextCharacter = source[i];
				if (nextCharacter == '"' && source[i - 1] != '\\') { //String delimiter found, since previous char isn't a escape character
					++i;
					break;
				}
			}
			token = std::string_view(&source[start], i - start);

			return;
		}


		const std::vector<uint8_t>& delimiters = { ',', ':', '{', '}', '[', ']', ' ', '\r', '\n', '\t', '\v', '\f' }; //Whitespace included
		SubstringOnCharacter(std::string_view(&source[i], source.size() - i), token, delimiters);
		i += token.size();
		return;
	}

}


bool JSON::SubstringOnCharacter(const std::string_view& string, std::string_view& substring, const std::vector<uint8_t>& characters) {

	for (size_t i = 0; i < string.size(); ++i) {
		const uint8_t character = string[i];
		for (const uint8_t c : characters) {
			if (character == c) {
				if (i == 0) //Avoids substringing nothing if the first character is a delimiter
					i = 1;

				substring = std::string_view(&string[0], i);
				return true;
			}
		}
	}

	substring = std::string_view(&string[0], string.size());
	return false;
}
