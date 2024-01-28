#pragma once

#include <string>

using namespace std;

namespace ZZCollect
{

class CollectConf
{
public:
    CollectConf();

public:
    int    vacantMobileNumberWorkerPoolCoreSize = 7;
    int    vacantMobileNumberWorkerPoolMaxSize = 7;
    bool   vacantMobileSwitch = true;
};

class CommonConf
{
public:
    int    resId;
    string taskCenterAddress;
    string pushCenterAddress;
    string collectorId;
    string larkAlarmUrl;  // lark_alarm_url="https://open.feishu.cn/open-apis/bot/v2/hook/61fff032-474a-4ff4-9256-bb99fa2a72c1"
};

class GlobalConf
{
public:
    CollectConf collectConf;
    CommonConf  commonConf;
};

bool readConfig(const char *file, GlobalConf& globalConf);

}
