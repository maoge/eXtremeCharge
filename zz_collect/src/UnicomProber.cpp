#include <nlohmann/json.hpp>
#include <iostream>

#include "Base64.h"
#include "UnicomProber.h"
#include "GlobalRes.h"
#include "HttpClient.h"
#include "ChinaTelecomTools.h"
#include "ChinaUnicomTools.h"

using namespace ZZTools;

namespace ZZCollect
{

extern GlobalRes GLOBAL_RES;
static const string PUMP_BODY = "{\"operator\":0}";

void UnicomProber::Run()
{
    while (IsContinue()) {
        string rawData = pumpData();
        if (rawData.empty()) {
            sleep(VACANT_RETRY_INTERVAL);
            continue;
        }
        
        checkMigrate(rawData);
    }
}

string UnicomProber::pumpData()
{
    HttpClient httpClient;
    string strResponse;

    map<string, string> headers;
    headers.insert(make_pair<>("Content-Type", "application/json"));

    for (int i = 0; i < VACANT_MAX_RETRY; i++)
    {
        if (httpClient.Post(m_sTaskCenterAddr, headers, PUMP_BODY, 5, 5, strResponse))
        {
            logger->info(LTRACE, "pumpData: %s", strResponse.c_str());
            break;
        }
        else
        {
            logger->error(LTRACE, "pumpData NOK, url: %s, retry ....", m_sTaskCenterAddr.c_str());
            sleep(VACANT_RETRY_INTERVAL);
        }
    }

    return strResponse;
}

void UnicomProber::notifyResult(cJSON* resultJson)
{
    HttpClient httpClient;
    string strResponse;

    char* buff = cJSON_PrintUnformatted(resultJson);
    string notifyData(buff);

    map<string, string> headers;
    headers.insert(make_pair<>("Content-Type", "application/json"));

    for (int i = 0; i < VACANT_MAX_RETRY; i++)
    {
        if (httpClient.Post(m_sPushCenterAddr, headers, notifyData, 5, 5, strResponse))
        {
            logger->info(LTRACE, "notifyResult: %s", notifyData.c_str());
            break;
        }
        else
        {
            logger->error(LTRACE, "notifyResult NOK, url: %s, retry ....", m_sPushCenterAddr.c_str());
            sleep(VACANT_RETRY_INTERVAL);
        }
    }

    cJSON_free(buff);
}

bool UnicomProber::checkMigrate(string& rawData)
{
    cJSON* pRoot = cJSON_Parse(rawData.c_str());
    cJSON* pItem = cJSON_GetObjectItem(pRoot, HEADER_RET_CODE);

    int retCode = pItem->valueint;
    if (retCode != RET_CODE_OK)
    {
        sleep(VACANT_RETRY_INTERVAL);
        cJSON_Delete(pRoot);
        return false;
    }
    
    m_taskCounter.IncrementAndGet();
    cJSON* resultJson = cJSON_CreateObject();

    cJSON* pRetInfo = cJSON_GetObjectItem(pRoot, HEADER_RET_INFO);
    string strPhone = cJSON_GetObjectItem(pRetInfo, "phone")->valuestring;
    int phoneStatus = cJSON_GetObjectItem(pRetInfo, "status")->valueint;
    string sec = strPhone.substr(3, 5);
    cJSON_Delete(pRoot);
    
    int realOperator = ChinaTelecomTools::getPhoneOperator(strPhone);
    logger->info(LTRACE, "real operator: %d", realOperator);
    cJSON_AddItemToObject(resultJson, "phone", cJSON_CreateString(strPhone.c_str()));

    // 移动低位段号码基本都是空号用于移动早期自己内部信令测试
    if (realOperator == OPERATOR_TYPE_CMCC && sec == "00") {
        logger->info(LTRACE, "low section moved phone: %s, save vacant .... ", strPhone.c_str());
        cJSON_AddItemToObject(resultJson, "status", cJSON_CreateNumber(PHONE_STATUS_VACANT_NUM));

        notifyResult(resultJson);
        cJSON_Delete(resultJson);
        return true;
    }

    switch (realOperator) {
    case OPERATOR_TYPE_UNKNOW:
        cJSON_AddItemToObject(resultJson, "status", cJSON_CreateNumber(PHONE_STATUS_VACANT_NUM));
        break;
    case OPERATOR_TYPE_UNICOM:
        {
            m_callCounter.IncrementAndGet();
            int uniStatus = ChinaUnicomTools::checkUnicomPhoneVacant(strPhone);
            cJSON_AddItemToObject(resultJson, "operator", cJSON_CreateNumber(realOperator));
            cJSON_AddItemToObject(resultJson, "status", cJSON_CreateNumber(uniStatus));
            
        }
        break;
    case OPERATOR_TYPE_TELECOM:
        cJSON_AddItemToObject(resultJson, "operator", cJSON_CreateNumber(realOperator));
        cJSON_AddItemToObject(resultJson, "status", cJSON_CreateNumber(PHONE_MOVED));
        break;
    case OPERATOR_TYPE_CMCC:
        cJSON_AddItemToObject(resultJson, "operator", cJSON_CreateNumber(realOperator));
        cJSON_AddItemToObject(resultJson, "status", cJSON_CreateNumber(PHONE_MOVED));
        break;
    default:
        break;
    }

    logger->info(LTRACE, "task count: %d, unicom call: %d", m_taskCounter.Get(), m_callCounter.Get());
    notifyResult(resultJson);

    if (realOperator == OPERATOR_TYPE_UNICOM) {
        sleep(FETCH_INTERVAL);  // 防止频繁请求联通网厅触发流控
    }

    cJSON_Delete(resultJson);

    return true;
}

}
