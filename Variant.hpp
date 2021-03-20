#pragma once

#include <memory>
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


	Variant() {
		*(int64_t*)&ptr = 0;
		type = Int;
	}


	Variant(void* pData) {
		ptr = std::make_shared<void*>(pData);
		type = Pointer;
	}

	Variant(bool pData) {
		*(bool*)&ptr = pData;
		type = Bool;
	}

	Variant(int64_t pData) {
		*(int64_t*)&ptr = pData;
		type = Int;
	}

	Variant(double pData) {
		*(double*)&ptr = pData;
		type = Float;
	}

	Variant(std::string pData) {
		ptr = std::make_shared<std::string>(pData);
		type = String;
	}


	Variant(std::vector<void*> pData) {
		ptr = std::make_shared<std::vector<void*>>(pData);
		type = PointerArray;
	}

	Variant(std::vector<bool> pData) {
		ptr = std::make_shared<std::vector<bool>>(pData);
		type = BoolArray;
	}

	Variant(std::vector<uint8_t> pData) {
		ptr = std::make_shared<std::vector<uint8_t>>(pData);
		type = ByteArray;
	}

	Variant(std::vector<int64_t> pData) {
		ptr = std::make_shared<std::vector<int64_t>>(pData);
		type = IntArray;
	}

	Variant(std::vector<double> pData) {
		ptr = std::make_shared<std::vector<double>>(pData);
		type = FloatArray;
	}

	Variant(std::vector<std::string> pData) {
		ptr = std::make_shared<std::vector<std::string>>(pData);
		type = StringArray;
	}


	Variant(std::vector<Variant> pData) {
		ptr = std::make_shared<std::vector<Variant>>(pData);
		type = VariantArray;
	}

	Variant(std::unordered_map<std::string, Variant> pData) {
		ptr = std::make_shared<std::unordered_map<std::string, Variant>>(pData);
		type = Dictionary;
	}

	Variant(std::unordered_map<std::string, Variant>* pData) { //Variant will hold a non-owned pointer
		*(std::unordered_map<std::string, Variant>**)&ptr = pData;
		type = Dictionary;
	}


	inline void* GetData() const {
		return ptr.get();
	}

	inline Variant::Type GetType() const {
		return type;
	}


	operator void*() const {
		if (type == Pointer || type == Int) {
			return (void*)*(uintptr_t*)ptr.get();
		}

		return nullptr;
	}

	operator bool() const {
		if (type == Bool)
			return *(bool*)&ptr;

		return false;
	}

	operator int64_t() const {
		if (type == Float) //Must be casted first for correct conversion //NOTE: Rounds the double first, else the decimal part would get truncated/discarded (Example: 7.9 -> 7 instead of 8)
			return (int64_t)std::round(*(double*)&ptr);
		if (type == Int)
			return *(int64_t*)&ptr;

		return 0;
	}

	operator double() const {
		if (type == Int) //Must be casted first for correct conversion
			return (double)*(int64_t*)&ptr;
		if (type == Float)
			return *(double*)&ptr;

		return 0.0;
	}


	operator std::string() const {

		if (type == String)
			return *(std::string*)ptr.get();

		if (type == Bool) {
			if (*(bool*)ptr.get())
				return "true";
			return "false";
		}

		if (type == Int)
			return std::to_string(*(int64_t*)ptr.get());

		if (type == Float)
			return std::to_string(*(double*)ptr.get());

		return "";
	}


	operator std::vector<void*>() const {
		if (type == PointerArray) {
			return *(std::vector<void*>*)ptr.get();
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr.get();
			std::vector<void*> converted;
			converted.resize(thisVector.size());
			for (uint32_t i = 0; i < thisVector.size(); ++i) {
				converted[i] = thisVector[i].ptr.get();
			}

			return converted;
		}

		return std::vector<void*>();
	}

	operator std::vector<bool>() const {
		if (type == BoolArray) {
			return *(std::vector<bool>*)ptr.get();
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr.get();
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
			return *(std::vector<uint8_t>*)ptr.get();
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr.get();
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
			return *(std::vector<int64_t>*)ptr.get();
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr.get();
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
			return *(std::vector<double>*)ptr.get();
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr.get();
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
			return *(std::vector<std::string>*)ptr.get();
		}

		if (type == VariantArray) {
			const auto& thisVector = *(std::vector<Variant>*)ptr.get();
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
			return *(std::vector<Variant>*)ptr.get();

		return std::vector<Variant>();
	}

	operator std::unordered_map<std::string, Variant>() const {
		if (type == Dictionary)
			return *(std::unordered_map<std::string, Variant>*)ptr.get();

		return std::unordered_map<std::string, Variant>();
	}

private:

	//NOTE: When making a void shared_ptr the ptr stores the type deleter and memory should be freed as expected
	std::shared_ptr<void> ptr; //This value is also used to store primitive types as if it was an union, it should be safe
	Type type;

};