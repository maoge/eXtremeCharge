#pragma once

#include <lwpr.h>
#include "CollectFixDef.h"

using namespace std;

namespace ZZCollect
{

class MobileMigrateProber : public ZZTools::Thread {
public:
    MobileMigrateProber(const string& sTaskCenterAddr, const string& sPushCenterAddr, const string& sProberName);
    virtual ~MobileMigrateProber();

    void Run();

    void checkVacantPhoneTask(const VacantTask& vacantTask, vector<VacantResult>& result);
    int  checkVacantPhone(const string& phone, const string& prov, const string& ck);

private:
    void doNotify(const vector<MigrateResult>& resultVec);

private:
    string m_sTaskCenterAddr;
    string m_sPushCenterAddr;
    string m_sProberName;
};

}
