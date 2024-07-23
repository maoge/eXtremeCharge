#include <string>
#include <iostream>

#include "lwpr.h"
#include "GlobalRes.h"
#include "MobileMigrateProber.h"

using namespace std;
using namespace ZZTools;
using namespace ZZCollect;

namespace ZZCollect
{
GlobalRes GLOBAL_RES;
vector<MobileMigrateProber*> PROBER_VEC;
}

void exitHandler(int signum)
{
    // release global resource
    logger->info(LTRACE, "zz_collect exit signum: %d", signum);
    GLOBAL_RES.isRunning = false;

    vector<MobileMigrateProber*>::iterator it = PROBER_VEC.begin();
    vector<MobileMigrateProber*>::iterator end = PROBER_VEC.end();
    for (; it != end; it++)
    {
        MobileMigrateProber* prober = *it;
        prober->StopRunning();

        delete prober;
    }
    PROBER_VEC.clear();
}

bool initLogger()
{
    string logName = ZZTools::ProcessUtil::GetPName() + ".log";
    string logFile = FileUtil::BuildPath("./log/") + logName;

    LOGGER_OPTION_T optLogger;
    optLogger.strLogFileName = logFile;
    optLogger.nLogLevel      = ZZTools::LOGGER_LEVEL_INFO;
    optLogger.bPrintWhere    = true;
    optLogger.bPrintFile     = true;
    optLogger.bPrintConsole  = true;
    logger->Initialize(optLogger);

    return true;
}

bool init()
{
    assert(true == initLogger());
    assert(true == GLOBAL_RES.initConf());

    return true;
}

void mainLoop() {
    while (GLOBAL_RES.isRunning)
    {
        VacantProberTool::refreshVacantSetting();
        sleep(20);
    }
}

void startWorker()
{
    const CollectConf& collectConf = GLOBAL_RES.globalConf.collectConf;
    bool bVacantMobileSwitch = collectConf.vacantMobileSwitch;

    const CommonConf& commonConf = GLOBAL_RES.globalConf.commonConf;
    string sTaskCenterAddress = commonConf.taskCenterAddress;
    string sPushCenterAddress = commonConf.pushCenterAddress;

    if (bVacantMobileSwitch)
    {
        int workerCoreSize = collectConf.vacantMobileNumberWorkerPoolCoreSize;
        for (int i = 0; i < workerCoreSize; i++)
        {
            char buff[256] = {0};
            snprintf(buff, 255, "mobile_prober_%02d", (i + 1));
            string proberName = string(buff);

            MobileMigrateProber* prober = new MobileMigrateProber(sTaskCenterAddress, sPushCenterAddress, proberName);
            prober->Start();

            PROBER_VEC.push_back(prober);
            sleep(1);
        }
    }
}

int main(void)
{
    logger->info(LTRACE, "mobile_migrate_main start pid: %d", ProcessUtil::GetPID());

    ZZTools::Utility::RegisterSignal(SIGINT, exitHandler);
    ZZTools::Utility::RegisterSignal(SIGTERM, exitHandler);

    assert(true == init());

    startWorker();
    mainLoop();
}
