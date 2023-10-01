#pragma once

#include <string>

#include "CollectFixDef.h"

using namespace std;

namespace ZZCollect
{

class VacantProberTool {
public:
	VacantProberTool() { }
	virtual ~VacantProberTool() { }

	static bool fetchVacantNumTask(const string& centerAddr, const string& prov, int oper, int resId, VacantTask& vacantTask);
	static bool notifyLarkAlarm(int alarmType, const string& alarmInfo);
	static bool notifyCheckCKInvalid(int resId);
	static bool sendLarkTextMsg(const string& botHookUrl, const string& msg);

	static void refreshVacantSetting();

};

}
