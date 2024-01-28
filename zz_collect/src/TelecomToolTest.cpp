#include <string>
#include <iostream>
#include <lwpr.h>

#include "ChinaTelecomTools.h"

using namespace std;
using namespace ZZTools;

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

    return true;
}

int main(void)
{
    assert(true == init());

    string phone("13799402330");

    int phoneOperator = ZZCollect::ChinaTelecomTools::getPhoneOperator(phone);
    cout << "phone operator:" << phoneOperator << endl;
}
