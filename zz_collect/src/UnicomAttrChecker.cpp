#include <string>
#include <iostream>
#include <base64/base64.h>
#include <lwpr.h>

#include "Cipher.h"
#include "GlobalRes.h"
#include "UnicomAttrProber.h"

using namespace std;
using namespace ZZTools;
using namespace ZZCollect;

namespace ZZCollect
{
GlobalRes GLOBAL_RES;
UnicomAttrProber* P_PROBER;
}

void exitHandler(int signum)
{
    // release global resource
    logger->info(LTRACE, "unicom_attr_checker exit signum: %d", signum);
    GLOBAL_RES.isRunning = false;

    P_PROBER->StopRunning();
    delete P_PROBER;
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
        sleep(20);
    }
}

void startWorker()
{
    const CommonConf& commonConf = GLOBAL_RES.globalConf.commonConf;
    string sTaskCenterAddress = "http://" + commonConf.taskCenterAddress + "/api/collect/rpopPhoneAttributeResult";
    string sPushCenterAddress = "http://" + commonConf.pushCenterAddress + "/api/collect/rpushMigratePhone2";

    string proberName = "unicom_attr_checker";
    P_PROBER = new UnicomAttrProber(sTaskCenterAddress, sPushCenterAddress, proberName);
    P_PROBER->Start();
}

int main(void)
{
    logger->info(LTRACE, "unicom_attr_checker start pid: %d", ProcessUtil::GetPID());

    ZZTools::Utility::RegisterSignal(SIGINT, exitHandler);
    ZZTools::Utility::RegisterSignal(SIGTERM, exitHandler);

    assert(true == init());

    startWorker();
    mainLoop();
}
