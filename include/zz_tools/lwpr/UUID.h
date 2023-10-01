#pragma once

#include <stdlib.h>

namespace ZZTools
{

#pragma pack(push)
#pragma pack(4)
	typedef union {
		unsigned char data[32];
		struct {
			unsigned char group1[8];
			unsigned char group2[4];
			unsigned char group3[4];
			unsigned char group4[4];
			unsigned char group5[12];
		} groups;
} UUID_T;
#pragma pack(pop)

	class UUID {
	public:
		UUID(void);
		virtual ~UUID(void);
		UUID(const UUID&);
		explicit UUID(const std::string&);
		UUID& operator=(const UUID&);
		
		bool isNil(void) const;
		unsigned char getVersion(void) const;
		std::string toString(void) const;
		static std::string getRandomUUIDString(void);
	
	private:
		UUID_T _data;
		
		void createRandomUUID(void);
		int  nextID();
	};

};
