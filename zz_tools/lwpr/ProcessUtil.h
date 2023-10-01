#pragma once

#include "LWPRType.h"
#include "Buffer.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <string>

namespace ZZTools
{
	class ProcessUtil
	{
	public:
		static PID_T MakeProcess(const char* cmd);
		static PID_T MakeProcess(const std::vector<std::string>& cmd);
		static PID_T GetPID();
		static std::string GetPName();
		static bool IsProcessExist(PID_T pid);
		static std::string GetWorkingDir();

		static bool GetProcessCmdline(std::vector<std::string>& cmd);

		static bool GetProcessCmdline(PID_T pid, std::vector<std::string>& cmd);

	private:
	};
};
