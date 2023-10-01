#include "lwpr.h"

#include "VacantProberTool.h"
#include "HttpClient.h"
#include "GlobalRes.h"

using namespace ZZTools;

namespace ZZCollect
{

extern GlobalRes GLOBAL_RES;

map<int, long> ALARM_DEFEND_MAP;

static string LARK_ALARM_URL = "https://open.feishu.cn/open-apis/bot/v2/hook/61fff032-474a-4ff4-9256-bb99fa2a72c1";

bool VacantProberTool::fetchVacantNumTask(const string& centerAddr, const string& prov, int oper, int resId, VacantTask& vacantTask)
{
    bool res = false;
    
    char buff[256] = {0};
    snprintf(buff, 255, "http://%s/api/collect/fetchVacantNumTaskPerProv?operator=%d&prov=%s&resId=%d",
            centerAddr.c_str(), oper, prov.c_str(), resId);
    
    string url = string(buff);
    string respBody;
    HttpClient httpClient;

    int resCode = httpClient.Get(url, VACANT_CONN_TIMEOUT, VACANT_READ_TIMEOUT, respBody);
    if (resCode < 0)
    {
        return false;
    }

    cJSON* pRoot = cJSON_Parse(respBody.c_str());
    cJSON* pItem = cJSON_GetObjectItem(pRoot, HEADER_RET_CODE);

    int retCode = pItem->valueint;
    if (retCode == RET_CODE_OK)
    {
        pItem = cJSON_GetObjectItem(pRoot, HEADER_RET_INFO);
        if (pItem != NULL)
        {
            vacantTask.oper      = cJSON_GetObjectItem(pItem, "operator")->valueint;
            vacantTask.prov      = cJSON_GetObjectItem(pItem, "prov")->valuestring;
            vacantTask.ck        = cJSON_GetObjectItem(pItem, "ck")->valuestring;
            vacantTask.phoneList = cJSON_GetObjectItem(pItem, "phoneList")->valuestring;
        }
        res = true;
    }
    else if (retCode == RET_CODE_NOK)
    {
        pItem = cJSON_GetObjectItem(pRoot, HEADER_RET_INFO);
        logger->error(LTRACE, "fetchVacantNumTask response: %s, prov: %s, operator: %d, resId: %d",
                                pItem->valuestring, prov.c_str(), oper, resId);
    }

    cJSON_Delete(pRoot);
    return res;
}

bool VacantProberTool::notifyLarkAlarm(int alarmType, const string& alarmInfo)
{
    map<int, long>::iterator it = ALARM_DEFEND_MAP.find(alarmType);
    long currTime = Utility::GetMillisecondsFromLocal();
    if (it != ALARM_DEFEND_MAP.end())
    {
        // 告警还在窗口内则跳过
        long alarmExpire = it->second;
        if (currTime < alarmExpire) {
            logger->info(LTRACE, "lark alarm skip, %s", alarmInfo.c_str());
            return true;
        } else {
            ALARM_DEFEND_MAP.insert(pair<int, long>(alarmType, currTime + ALARM_TTL));
        }
    }
    else
    {
        ALARM_DEFEND_MAP.insert(pair<int, long>(alarmType, currTime + ALARM_TTL));
    }
	
    bool result = sendLarkTextMsg(LARK_ALARM_URL, alarmInfo);
    if (result) {
		logger->info(LTRACE, "lark alarm send ok, %s", alarmInfo.c_str());
	} else {
		logger->error(LTRACE, "lark alarm send nok, %s", alarmInfo.c_str());
	}
    return result;
}

bool VacantProberTool::notifyCheckCKInvalid(int resId)
{
    static string NOTIFY_URL = "http://" + GLOBAL_RES.globalConf.commonConf.taskCenterAddress + "/api/collect/notifyCheckCKInvalid";

    cJSON* pRoot = cJSON_CreateObject();
    cJSON_AddItemToObject(pRoot, "resId", cJSON_CreateNumber(resId));

    char* buff = cJSON_PrintUnformatted(pRoot);
    string reqData(buff);

    cJSON_free(buff);
    cJSON_Delete(pRoot);

    bool result = false;

    for (int i = 0; i < VACANT_MAX_RETRY; i++)
    {
        HttpClient httpClient;
        string strResponse;
        if (httpClient.Post(NOTIFY_URL, reqData, 5, 5, strResponse))
        {
            logger->info(LTRACE, "notifyCheckCKInvalid:%s OK ......", NOTIFY_URL.c_str());
            result = true;
            break;
        }
        else
        {
            logger->info(LTRACE, "notifyCheckCKInvalid:%s NOK ......", NOTIFY_URL.c_str());
            sleep(VACANT_RETRY_INTERVAL);
        }
    }

    return result;
}

bool VacantProberTool::sendLarkTextMsg(const string& botHookUrl, const string& msg)
{
    // {"msg_type":"text","content":{"text":"@所有人 request example"}}

    cJSON* pContent = cJSON_CreateObject();
    cJSON_AddItemToObject(pContent, "text", cJSON_CreateString(msg.c_str()));

    cJSON* pRoot = cJSON_CreateObject();
    cJSON_AddItemToObject(pRoot, "msg_type", cJSON_CreateString("text"));
    cJSON_AddItemToObject(pRoot, "content", pContent);

    map<string, string> headers;
    headers.insert(pair<string, string>("Accept", "application/json, text/plain, */*"));
    headers.insert(pair<string, string>("Accept-Language", "zh-CN"));
    headers.insert(pair<string, string>("Connection", "keep-alive"));

    char* buff = cJSON_PrintUnformatted(pRoot);
    string strPost(buff);

    cJSON_free(buff);
    cJSON_Delete(pRoot);

    bool result = false;
    for (int i = 0; i < VACANT_MAX_RETRY; i++)
    {
        // {"StatusCode":0,"StatusMessage":"success"}
        HttpClient httpClient;
        string strResponse;
        if (!httpClient.PostSSL(botHookUrl, headers, strPost, NULL, 5, 5, strResponse))
        {
            sleep(VACANT_RETRY_INTERVAL);
            continue;
        }

        if (strResponse.empty())
        {
            logger->error(LTRACE, "sendLarkTextMsg fail, response null");
            continue;
        }

        result = strResponse.find("success") != string::npos;
        break;
    }

    return result;
}

void VacantProberTool::refreshVacantSetting() {
    static string VACANT_SETTING_URL = "http://" + GLOBAL_RES.globalConf.commonConf.taskCenterAddress + "/api/collect/getVacantCollectSetting";
    
    for (int i = 0; i < VACANT_MAX_RETRY; i++)
    {
        HttpClient httpClient;
        string strResponse;
        if (!httpClient.Post(VACANT_SETTING_URL, "", 5, 5, strResponse))
        {
            logger->error(LTRACE, "call getVacantCollectSetting fail, retry ......");
            sleep(VACANT_RETRY_INTERVAL);
            continue;
        }

        if (strResponse.empty())
        {
            logger->error(LTRACE, "getVacantCollectSetting result NOK, retry ......");
            sleep(VACANT_RETRY_INTERVAL);
            continue;
        }

        logger->debug(LTRACE, "getVacantCollectSetting response: %s", strResponse.c_str());

        cJSON* pRoot = cJSON_Parse(strResponse.c_str());
        cJSON* pItem = cJSON_GetObjectItem(pRoot, HEADER_RET_CODE);

        int retCode = pItem->valueint;
        if (retCode != RET_CODE_OK)
        {
            cJSON_Delete(pRoot);
            sleep(VACANT_RETRY_INTERVAL);
            continue;
        }

        cJSON* pRetInfo = cJSON_GetObjectItem(pRoot, HEADER_RET_INFO);
        GLOBAL_RES.prov = cJSON_GetObjectItem(pRetInfo, "prov")->valuestring;
        GLOBAL_RES.vacantEnabled = cJSON_IsTrue(cJSON_GetObjectItem(pRetInfo, "vacantEnable"));

        cJSON_Delete(pRoot);

        break;
    }
}


}
