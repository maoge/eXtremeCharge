#include <sstream>
#include <iostream>
#include <string.h>
#include "UUID.h"

namespace ZZTools
{

#define __32_ZEROS__ "00000000000000000000000000000000"
static const unsigned char _TABLE[4] = {'8', '9', 'a', 'b'};

	UUID::UUID(void) {
		createRandomUUID();
	}

	UUID::~UUID(void) {
		
	}
	
	UUID::UUID(const UUID& o) {
		memcpy(_data.data, o._data.data, sizeof(_data));
	}

	UUID::UUID(const std::string& uuidString) {
		std::stringstream stringStream(uuidString);
		std::string subString;
		unsigned int size = sizeof(_data.data), i = 0, j, loops;
		while(std::getline(stringStream, subString, '-')) {
			loops = subString.length();
			for (j = 0; j < loops; j++) {
				_data.data[i++] = subString.at(j);
				if (i == size) break;
			}
			if (i == size) break;
		}
	}

	UUID& UUID::operator=(const UUID& other) {
		if (this != &other)
			memcpy(_data.data, other._data.data, sizeof(_data));
		return *this;
	}

	bool UUID::isNil(void) const {
		unsigned int loops = sizeof(_data.data);
		for (unsigned int i = 0; i < loops; i++)
			if (_data.data[i] != '0') return false;
		return true;
	}

	unsigned char UUID::getVersion(void) const {
		return _data.groups.group3[0];
	}

	std::string UUID::toString(void) const {
		std::string result;
		std::stringstream stringStream;
		unsigned int loops = sizeof(_data.groups.group1), i;
		for (i = 0; i < loops; i++)
			stringStream << _data.groups.group1[i];
		stringStream << '-';
		loops = sizeof(_data.groups.group2);
		for (i = 0; i < loops; i++)
			stringStream << _data.groups.group2[i];
		stringStream << '-';
		loops = sizeof(_data.groups.group3);
		for (i = 0; i < loops; i++)
			stringStream << _data.groups.group3[i];
		stringStream << '-';
		loops = sizeof(_data.groups.group4);
		for (i = 0; i < loops; i++)
			stringStream << _data.groups.group4[i];
		stringStream << '-';
		loops = sizeof(_data.groups.group5);
		for (i = 0; i < loops; i++)
			stringStream << _data.groups.group5[i];
		stringStream >> result;
		return result;
	}

	std::string UUID::getRandomUUIDString(void) {
		return UUID().toString();
	}

	void UUID::createRandomUUID(void) {
		std::stringstream stringStream;
		std::string numberString;
		stringStream << std::hex << nextID();
		stringStream << std::hex << nextID();
		stringStream << std::hex << nextID();
		stringStream << std::hex << nextID();
		stringStream << __32_ZEROS__;
		stringStream >> numberString;
		unsigned int loops = sizeof(_data.data);
		numberString = numberString.substr(0, loops);

		for (unsigned int i = 0; i < loops; i++)
			_data.data[i] = numberString.at(i);
		_data.groups.group3[0] = '4';
		_data.groups.group4[0] = _TABLE[nextID()%4];
	}
	
	int UUID::nextID() {
		unsigned int seed = (unsigned)time(NULL) + rand();
		return rand_r(&seed);
	}
	
};
