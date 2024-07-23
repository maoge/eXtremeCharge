#include <nlohmann/json.hpp>
#include <iostream>

#include "Base64.h"
#include "UnicomAttrProber.h"
#include "GlobalRes.h"
#include "HttpClient.h"
#include "ChinaUnicomTools.h"

using namespace ZZTools;

namespace ZZCollect
{

extern GlobalRes GLOBAL_RES;
static const string PUMP_BODY = "{\"operator\":0}";

void UnicomAttrProber::Run()
{
    while (IsContinue()) {
        string rawData = pumpData();
        if (rawData.empty()) {
            sleep(VACANT_RETRY_INTERVAL);
            continue;
        }
        
        checkAttr(rawData);
    }
}

string UnicomAttrProber::pumpData()
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

void UnicomAttrProber::notifyResult(cJSON* resultJson)
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

bool UnicomAttrProber::checkAttr(string& rawData)
{
    cJSON* pRoot = cJSON_Parse(rawData.c_str());
    cJSON* pRetCode = cJSON_GetObjectItem(pRoot, HEADER_RET_CODE);

    int retCode = pRetCode->valueint;
    if (retCode != RET_CODE_OK)
    {
        cJSON_Delete(pRoot);
        sleep(FETCH_INTERVAL);
        return false;
    }

    // [{"phone":"13510316981","operator":-1,"ori_operator":2}]
    // [{"phone":"13512102755","operator":-1,"ori_operator":2},{"phone":"13512111100","operator":-1,"ori_operator":2},{"phone":"13512059062","operator":-1,"ori_operator":2}]

    cJSON* pRetInfo = cJSON_GetObjectItem(pRoot, HEADER_RET_INFO);
    if (!pRetInfo)
    {
        cJSON_Delete(pRoot);
        sleep(FETCH_INTERVAL);
        return false;
    }

    m_taskCounter.IncrementAndGet();

    int size = cJSON_GetArraySize(pRetInfo);
    for (int idx = 0; idx < size; idx++) {
        cJSON* pItem = cJSON_GetArrayItem(pRetInfo, idx);

        string strPhone = cJSON_GetObjectItem(pItem, "phone")->valuestring;
        int oriOperator = cJSON_GetObjectItem(pItem, "ori_operator")->valueint;
        
        int realOperator = ChinaUnicomTools::getPhoneOperator(strPhone);

        cJSON* resultJson = cJSON_CreateObject();
        cJSON_AddItemToObject(resultJson, "phone", cJSON_CreateString(strPhone.c_str()));
        cJSON_AddItemToObject(resultJson, "operator", cJSON_CreateNumber(realOperator));
        notifyResult(resultJson);
        cJSON_Delete(resultJson);

        logger->info(LTRACE, "real operator: %d", realOperator);
        logger->info(LTRACE, "task count: %d, unicom call: %d", m_taskCounter.Get(), m_callCounter.IncrementAndGet());
        
        sleep(FETCH_INTERVAL);  // 防止频繁请求联通网厅触发流控
    }

    cJSON_Delete(pRoot);
    return true;
}

}
