#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Variant {

public:

	enum Type : uint8_t {
		Unknown,
		Pointer, //void*
		Bool, //bool
		Int, //int64_t
		Float, //double
		String, //std::string
		PointerArray, //std::vector<void*>
		BoolArray, //std::vector<bool>
		ByteArray, //std::vector<uint8_t>
		IntArray, //std::vector<int64_t>
		FloatArray, //std::vector<double>
		StringArray, //std::vector<std::string>
		VariantArray, //std::vector<Variant>
		Dictionary, //std::unordered_map<std::string, Variant>

		Max
	};


	Variant(const Variant& other) {
		other.CopyData(*this);
	}

	Variant& operator=(const Variant& other) {
		//Free original data, if any
		if (OwnsData())
			FreeHeapData();

		other.CopyData(*this);
		return *this;
	}


	Variant(Variant&& other) {
		//Move data and adquire ownership of it
		ptr = other.ptr;
		type = other.type;
		ownership = other.ownership;
		other.ownership = false;
	}

	Variant& operator=(Variant&& other) {
		//Free original data, if any
		if (OwnsData())
			FreeHeapData();

		//Move data and adquire ownership of it
		ptr = other.ptr;
		type = other.type;
		ownership = other.ownership;
		other.ownership = false;
		return *this;
	}


	Variant() {
		*(int64_t*)&ptr = 0;
		type = Int;
		ownership = false;
	}

	Variant(void* pData) {
		ptr = pData;
		type = Pointer;
		ownership = false;
	}

	Variant(bool pData) {
		*(bool*)&ptr = pData;
		type = Bool;
		ownership = false;
	}

	Variant(int32_t pData) {
		*(int64_t*)&ptr = pData;
		type = Int;
		ownership = false;
	}

	Variant(int64_t pData) {
		*(int64_t*)&ptr = pData;
		type = Int;
		ownership = false;
	}

	Variant(float pData) {
		*(double*)&ptr = pData;
		type = Float;
		ownership = false;
	}

	Variant(double pData) {
		*(double*)&ptr = pData;
		type = Float;
		ownership = false;
	}

	Variant(const char* pData) {
		ptr = new std::string(pData);
		type = String;
		ownership = true;
	}

	Variant(std::string pData) {
		ptr = new std::string(pData);
		type = String;
		ownership = true;
	}

	Variant(std::vector<void*> pData) {
		ptr = new std::vector<void*>(pData);
		type = PointerArray;
		ownership = true;
	}

	Variant(std::vector<bool> pData) {
		ptr = new std::vector<bool>(pData);
		type = BoolArray;
		ownership = true;
	}

	Variant(std::vector<uint8_t> pData) {
		ptr = new std::vector<uint8_t>(pData);
		type = ByteArray;
		ownership = true;
	}

	Variant(std::vector<int64_t> pData) {
		ptr = new std::vector<int64_t>(pData);
		type = IntArray;
		ownership = true;
	}

	Variant(std::vector<double> pData) {
		ptr = new std::vector<double>(pData);
		type = FloatArray;
		ownership = true;
	}

	Variant(std::vector<std::string> pData) {
		ptr = new std::vector<std::string>(pData);
		type = StringArray;
		ownership = true;
	}

	Variant(std::vector<Variant> pData) {
		ptr = new std::vector<Variant>(pData);
		type = VariantArray;
		ownership = true;
	}

	Variant(std::unordered_map<std::string, Variant> pData) {
		ptr = new std::unordered_map<std::string, Variant>(pData);
		type = Dictionary;
		ownership = true;
	}

	Variant(std::unordered_map<std::string, Variant>* pData) {
		*(std::unordered_map<std::string, Variant>**)&ptr = pData;
		type = Dictionary;
		ownership = false;
	}


	~Variant() {
		if (OwnsData())
			FreeHeapData();
	}


	inline void* GetData() const {
		return ptr;
	}

	inline Variant::Type GetType() const {
		return type;
	}

	inline bool OwnsData() const {
		return ownership;
	}


	//Copy Conversion operators //Should be safe no matter what

	operator void*() const {
		if (type == Pointer || type == Int) {
			return (void*)(uintptr_t)ptr;
		}

		return nullptr;
	}

	operator bool() const {
		if (type == Bool)
			return (bool)ptr;

		return false;
	}

	operator int64_t() const {
		if (type == Float) //Must be casted first for correct conversion //NOTE: Rounds the double first, else the decimal part would get truncated/discarded (Example: 7.9 -> 7 instead of 8)
			return (int64_t)std::round(*(double*)&ptr);
		if (type == Int)
			return (int64_t)ptr;

		return 0;
	}

	operator double() const {
		if (type == Int) //Must be casted first for correct conversion
			return *(double*)(int64_t*)&ptr;
		if (type == Float)
			return *(double*)&ptr;

		return 0.0;
	}


	operator std::string() const {

		if (type == String)
			return *(std::string*)ptr;

		if (type == Bool) {
			if (*(bool*)ptr)
				return "true";
			return "false";
		}

		if (type == Int)
			return std::to_string(*(int64_t*)ptr);

		if (type == Float)
			return std::to_string(*(double*)ptr);

		return "";
	}


	operator std::vector<void*>() const {
		if (type == PointerArray) {
			return *(std::vector<void*>*)ptr;
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr;
			std::vector<void*> converted;
			converted.resize(thisVector.size());
			for (uint32_t i = 0; i < thisVector.size(); ++i) {
				converted[i] = thisVector[i].ptr;
			}

			return converted;
		}

		return std::vector<void*>();
	}

	operator std::vector<bool>() const {
		if (type == BoolArray) {
			return *(std::vector<bool>*)ptr;
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr;
			std::vector<bool> converted;
			converted.resize(thisVector.size());
			for (uint32_t i = 0; i < thisVector.size(); ++i) {
				converted[i] = bool(thisVector[i]);
			}

			return converted;
		}

		return std::vector<bool>();
	}

	operator std::vector<uint8_t>() const {
		if (type == ByteArray) {
			return *(std::vector<uint8_t>*)ptr;
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr;
			std::vector<uint8_t> converted;
			converted.resize(thisVector.size());
			for (uint32_t i = 0; i < thisVector.size(); ++i) {
				converted[i] = (uint8_t)int64_t(thisVector[i]);
			}

			return converted;
		}

		return std::vector<uint8_t>();
	}

	operator std::vector<int64_t>() const {
		if (type == IntArray) {
			return *(std::vector<int64_t>*)ptr;
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr;
			std::vector<int64_t> converted;
			converted.resize(thisVector.size());
			for (uint32_t i = 0; i < thisVector.size(); ++i) {
				converted[i] = int64_t(thisVector[i]);
			}

			return converted;
		}

		return std::vector<int64_t>();
	}

	operator std::vector<double>() const {
		if (type == FloatArray) {
			return *(std::vector<double>*)ptr;
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr;
			std::vector<double> converted;
			converted.resize(thisVector.size());
			for (uint32_t i = 0; i < thisVector.size(); ++i) {
				converted[i] = double(thisVector[i]);
			}

			return converted;
		}

		return std::vector<double>();
	}

	operator std::vector<std::string>() const {
		if (type == StringArray) {
			return *(std::vector<std::string>*)ptr;
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr;
			std::vector<std::string> converted;
			converted.resize(thisVector.size());
			for (uint32_t i = 0; i < thisVector.size(); ++i) {
				converted[i] = std::string(thisVector[i]);
			}

			return converted;
		}

		return std::vector<std::string>();
	}

	operator std::vector<Variant>() const {
		if (type == VariantArray)
			return *(std::vector<Variant>*)ptr;

		return std::vector<Variant>();
	}

	operator std::unordered_map<std::string, Variant>() const {
		if (type == Dictionary)
			return *(std::unordered_map<std::string, Variant>*)ptr;

		return std::unordered_map<std::string, Variant>();
	}

private:

	void* ptr; //This value is also used to store primitive types as if it was an union
	struct {
		Type type : 7;
		bool ownership : 1;
	};


	inline void CopyData(Variant& toVariant) const {

		switch (type)
		{
		case Pointer: {
			toVariant.ptr = ptr;
			toVariant.type = type;
			toVariant.ownership = false;
			break;
		}

		case Bool: {
			toVariant.ptr = ptr;
			toVariant.type = type;
			toVariant.ownership = false;
			break;
		}

		case Int: {
			toVariant.ptr = ptr;
			toVariant.type = type;
			toVariant.ownership = false;
			break;
		}

		case Float: {
			toVariant.ptr = ptr;
			toVariant.type = type;
			toVariant.ownership = false;
			break;
		}

		case String: {
			toVariant.ptr = new std::string(*(std::string*)ptr);
			toVariant.type = type;
			toVariant.ownership = true;
			break;
		}

		case PointerArray: {
			toVariant.ptr = new std::vector<void*>(*(std::vector<void*>*)ptr);
			toVariant.type = type;
			toVariant.ownership = true;
			break;
		}

		case BoolArray: {
			toVariant.ptr = new std::vector<bool>(*(std::vector<bool>*)ptr);
			toVariant.type = type;
			toVariant.ownership = true;
			break;
		}

		case ByteArray: {
			toVariant.ptr = new std::vector<uint8_t>(*(std::vector<uint8_t>*)ptr);
			toVariant.type = type;
			toVariant.ownership = true;
			break;
		}

		case IntArray: {
			toVariant.ptr = new std::vector<int64_t>(*(std::vector<int64_t>*)ptr);
			toVariant.type = type;
			toVariant.ownership = true;
			break;
		}

		case FloatArray: {
			toVariant.ptr = new std::vector<double>(*(std::vector<double>*)ptr);
			toVariant.type = type;
			toVariant.ownership = true;
			break;
		}

		case StringArray: {
			toVariant.ptr = new std::vector<std::string>(*(std::vector<std::string>*)ptr);
			toVariant.type = type;
			toVariant.ownership = true;
			break;
		}

		case VariantArray: {
			toVariant.ptr = new std::vector<Variant>(*(std::vector<Variant>*)ptr);
			toVariant.type = type;
			toVariant.ownership = true;
			break;
		}

		case Dictionary: {
			toVariant.ptr = new std::unordered_map<std::string, Variant>(*(std::unordered_map<std::string, Variant>*)ptr);
			toVariant.type = type;
			toVariant.ownership = true;
			break;
		}

		default:
			break;
		}
	}

	inline void FreeHeapData() {

		switch (type) //We only care about heap allocated types
		{
		case String: {
			delete (std::string*)ptr;
			break;
		}

		case PointerArray: {
			delete (std::vector<void*>*)ptr;
			break;
		}

		case BoolArray: {
			delete (std::vector<bool>*)ptr;
			break;
		}

		case ByteArray: {
			delete (std::vector<uint8_t>*)ptr;
			break;
		}

		case IntArray: {
			delete (std::vector<int64_t>*)ptr;
			break;
		}

		case FloatArray: {
			delete (std::vector<double>*)ptr;
			break;
		}

		case StringArray: {
			delete (std::vector<std::string>*)ptr;
			break;
		}

		case VariantArray: {
			delete (std::vector<Variant>*)ptr;
			break;
		}

		case Dictionary: {
			delete (std::unordered_map<std::string, Variant>*)ptr;
			break;
		}

		default:
			break;
		}
	}

};
