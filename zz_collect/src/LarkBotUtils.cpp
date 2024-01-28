#include <lwpr.h>

#include "LarkBotUtils.h"
#include "HttpClient.h"
#include "CollectFixDef.h"

using namespace ZZTools;

namespace ZZCollect
{

bool LarkBotUtils::sendLarkTextMsg(const string& botHookUrl, const string& msg)
{
    // {"msg_type":"text","content":{"text":"@所有人 request example"}}
    cJSON* ptrText = cJSON_CreateObject();
    cJSON_AddItemToObject(ptrText, "text", cJSON_CreateString(msg.c_str()));

    cJSON* ptrPrams = cJSON_CreateObject();
    cJSON_AddItemToObject(ptrPrams, "msg_type", cJSON_CreateString("text"));
    cJSON_AddItemToObject(ptrPrams, "content", ptrText);

    char* buff = cJSON_PrintUnformatted(ptrPrams);
    string postData(buff);
    
    cJSON_free(buff);
    cJSON_Delete(ptrText);
    cJSON_Delete(ptrPrams);

    map<string, string> headers;
    headers.insert(make_pair<>("Accept", "application/json, text/plain, */*"));
    headers.insert(make_pair<>("Accept-Language", "zh-CN"));
    headers.insert(make_pair<>("Connection", "keep-alive"));
    
    bool result = false;
    HttpClient httpClient;

    for (int i = 0; i < VACANT_MAX_RETRY; i++) {
        string strResponse;
        if (!httpClient.PostSSL(botHookUrl, headers, postData, NULL, VACANT_CONN_TIMEOUT, VACANT_READ_TIMEOUT, strResponse)) {
            logger->error(LTRACE, "sendLarkTextMsg doPost error");
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }
        
        if (strResponse == "") {
            logger->error(LTRACE, "sendLarkTextMsg fail, response empty, retry");
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }
        
        // {"StatusCode":0,"StatusMessage":"success"}
        cJSON* ptrResultJson = cJSON_Parse(strResponse.c_str());
        cJSON* ptrStatusCode = cJSON_GetObjectItem(ptrResultJson, "StatusCode");
        if (ptrStatusCode == NULL) {
            logger->error(LTRACE, "sendLarkTextMsg response: %s", strResponse.c_str());
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }

        int statusCode = ptrStatusCode->valueint;
        if (statusCode != 0) {
            logger->error(LTRACE, "sendLarkTextMsg fail: %s", strResponse.c_str());
            sleep(INTERVAL_WHEN_ERR);
            continue;   
        }
            
        result = true;
        break;
    }
    
    return result;
}

}
