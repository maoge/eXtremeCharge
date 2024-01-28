#pragma once

#include <lwpr.h>
#include "CollectFixDef.h"

using namespace std;

namespace ZZCollect
{

class MobileVacantProber : public ZZTools::Thread {
public:
    MobileVacantProber(const string& sTaskCenterAddr, const string& sPushCenterAddr, const string& sProberName);
    virtual ~MobileVacantProber();

    void Run();

    void checkVacantPhoneTask(const VacantTask& vacantTask, vector<VacantResult>& result);
    int  checkVacantPhone(const string& phone, const string& prov, const string& ck);


    int  getPayRuleId(const string& prov);
    int  getProvId(const string& prov);

private:
    void doNotify(const vector<VacantResult>& vacantResultVec);

    static map<string, int> initProvMap();

private:
    string m_sTaskCenterAddr;
    string m_sPushCenterAddr;
    string m_sProberName;
};

}
