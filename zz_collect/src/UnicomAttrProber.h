#pragma once

#include <lwpr.h>
#include "CollectFixDef.h"

using namespace std;
using namespace ZZTools;

namespace ZZCollect
{

class UnicomAttrProber : public ZZTools::Thread {
public:
    UnicomAttrProber(const string& sTaskCenterAddr, const string& sPushCenterAddr, const string& sProberName)
        : m_sTaskCenterAddr(sTaskCenterAddr), m_sPushCenterAddr(sPushCenterAddr), m_sProberName(sProberName) {}
    virtual ~UnicomAttrProber() {}

    void Run();

    string pumpData();
    bool   checkAttr(string& rawData);
    void   notifyResult(cJSON* resultJson);

private:
    string m_sTaskCenterAddr;
    string m_sPushCenterAddr;
    string m_sProberName;

    AtomicInteger m_taskCounter;
    AtomicInteger m_callCounter;

};

}
