#pragma once

#include "LWPRType.h"
#include <map>
#include <vector>
#include <string>

namespace ZZTools
{
	class ProcessInfo
	{
	public:
		typedef std::map< std::string, std::string > PStatusMap;
		static bool GetProcessInfoByPID(PStatusMap& status, PID_T pid);

		static bool EnumCurrentUserProcess(PIDList& pids);

		static int GetProcessFDCount(PID_T pid);

	private:
	};
};
