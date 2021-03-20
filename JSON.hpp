#pragma once

#include "Variant.hpp"

class JSON {

public:

	static std::string ToJSON(const std::unordered_map<std::string, Variant>& map, bool prettyPrint = false);
	static std::unordered_map<std::string, Variant> ParseJSON(const std::string& string);

private:

	template <bool PrettyPrint>
	static void WriteValue(const Variant& variant, std::string& string, uint32_t indentation = 1);

	static void ParseValue(Variant& toContainer, const std::string& string, size_t& index);

	static void GetToken(const std::string_view& source, size_t& i, std::string_view& token);
	static bool SubstringOnCharacter(const std::string_view& string, std::string_view& substring, const std::vector<uint8_t>& characters);

	JSON() = delete;
};
