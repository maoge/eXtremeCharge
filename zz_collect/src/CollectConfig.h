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
};

class GlobalConf
{
public:
    CollectConf collectConf;
    CommonConf  commonConf;
};

bool readConfig(const char *file, GlobalConf& globalConf);

}
