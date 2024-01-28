#pragma once

#include <lwpr.h>
#include "CollectFixDef.h"

using namespace std;
using namespace ZZTools;

namespace ZZCollect
{

class UnicomProber : public ZZTools::Thread {
public:
    UnicomProber(const string& sTaskCenterAddr, const string& sPushCenterAddr, const string& sProberName)
        : m_sTaskCenterAddr(sTaskCenterAddr), m_sPushCenterAddr(sPushCenterAddr), m_sProberName(sProberName) {}
    virtual ~UnicomProber() {}

    void Run();

    string pumpData();
    bool   checkMigrate(string& rawData);
    void   notifyResult(cJSON* resultJson);

private:
    string m_sTaskCenterAddr;
    string m_sPushCenterAddr;
    string m_sProberName;

    AtomicInteger m_taskCounter;
    AtomicInteger m_callCounter;

};

}
