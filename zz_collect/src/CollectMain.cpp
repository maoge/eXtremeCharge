#include <string>
#include <iostream>
#include <base64/base64.h>

#include "lwpr.h"
#include "GlobalRes.h"
#include "MobileVacantProber.h"
#include "CollectFixDef.h"
#include "VacantProberTool.h"
#include "Cipher.h"

using namespace std;
using namespace ZZTools;
using namespace ZZCollect;

namespace ZZCollect
{
GlobalRes GLOBAL_RES;
vector<MobileVacantProber*> MOBILE_PROBER_VEC;
}

void exitHandler(int signum)
{
    // release global resource
    logger->info(LTRACE, "zz_collect exit signum: %d", signum);
    GLOBAL_RES.isRunning = false;

    vector<MobileVacantProber*>::iterator it = MOBILE_PROBER_VEC.begin();
    vector<MobileVacantProber*>::iterator end = MOBILE_PROBER_VEC.end();
    for (; it != end; it++)
    {
        MobileVacantProber* prober = *it;
        prober->StopRunning();

        delete prober;
    }
    MOBILE_PROBER_VEC.clear();
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

            MobileVacantProber* prober = new MobileVacantProber(sTaskCenterAddress, sPushCenterAddress, proberName);
            prober->Start();

            MOBILE_PROBER_VEC.push_back(prober);
            sleep(1);
        }
    }
}

int main(void)
{
    logger->info(LTRACE, "zz_collect start pid: %d", ProcessUtil::GetPID());

    ZZTools::Utility::RegisterSignal(SIGINT, exitHandler);
    ZZTools::Utility::RegisterSignal(SIGTERM, exitHandler);

    assert(true == init());

    startWorker();
    mainLoop();
    
    // static const std::string AES_KEY(CMCC_AES_KEY);
	// static const std::string AES_IV(CMCC_AES_IV);
    
	// string raw = "jsessionid-cmcc=n18A5C00C142C51C557A131284E556F49-1;jsessionid-cmcc=n18A5C00C142C51C557A131284E556F49-1;jsessionid-cmcc=n18A5C00C142C51C557A131284E556F49-1;jsessionid-cmcc=n18A5C00C142C51C557A131284E556F49-1;jsessionid-cmcc=n18A5C00C142C51C557A131284E556F49-1";
	// string aesEnc = ZZ_TOOLS::encrypt_cbc(raw, AES_KEY, AES_IV);
	// std::cout << "aesEnc:" << aesEnc << std::endl;

	// MobileVacantProber prober("", "", "");
    // int result = prober.checkVacantPhone("13829864870", "广东", "jsessionid-cmcc=nA8000364B602A087A0FE68D8F792E1FA-1");
    // std::cout << "result:" << result << std::endl;

    // string phone = "13829864870";

    // std::string skey(CMCC_AES_KEY);
    // std::string siv(CMCC_AES_IV);
    // std::string encryptedText = ZZ_TOOLS::encrypt_cbc(phone, skey, siv);
    // std::cout << "encryptedText:" << encryptedText << std::endl;

    // std::string base64EncData = base64_encode(encryptedText);
    // std::cout << "base64EncData:" << base64EncData << std::endl;

}

