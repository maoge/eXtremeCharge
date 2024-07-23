#include <cctype>
#include <algorithm>
#include <iostream>

#include <lwpr.h>

#include "ChinaTelecomTools.h"
#include "HttpClient.h"
#include "GlobalRes.h"
#include "CollectFixDef.h"
#include "Cipher.h"

using namespace ZZTools;

namespace ZZCollect
{

bool ChinaTelecomTools::applyLoginFactor(const string& phone, string& outPukKey)
{
    char buff[256] = { 0 };
    snprintf(buff, 255, LOGIN_PARAMS, phone.c_str(), Utility::GetMillisecondsFromLocal());
    string reqData(buff);

    HttpClient httpClient;
    string strResponse;

    for (int i = 0; i < VACANT_MAX_RETRY; i++) {
        if (httpClient.PostSSL(URL_LOGIN_FACTOR, reqData, NULL, VACANT_CONN_TIMEOUT, VACANT_READ_TIMEOUT, strResponse)) {
            break;
        }

        if (strResponse.find("<html>") != string::npos) {
            logger->error(LTRACE, "loginFactor response illegal: %s", strResponse.substr(0, 255).c_str());
        }

        strResponse = "";
        sleep(INTERVAL_WHEN_ERR);
    }
    
    if (strResponse == "") {
        logger->error(LTRACE, "loginFactor response illegal");
        return false;
    }

    // {"result":{"nonce":"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC8SYsLub0+A2M/ydBUU6ucwQOTDwy7I3+TMpRk73//l2eI6i4EbFm4TTGTTVoLdbu1PANKFZtADK9USJpC
    //            7qliW8LmhhEjHOaViJQD2eZTRtSbRLvro8SSzTM/ECAVlpGtZNEWYRbAWPVTrdX2swUIfweGNQRK1vmJV/4+svDvmQIDAQAB"},"success":true,"errorCode":null,"errorMsg":null}
    cJSON* pRoot = cJSON_Parse(strResponse.c_str());
    if (!cJSON_IsTrue(cJSON_GetObjectItem(pRoot, "success"))) {
        logger->error(LTRACE, "ChinaTelecomTools applyLoginFactor fail:%s", strResponse.c_str());
        cJSON_Delete(pRoot);
        return false;
    }
    
    cJSON* resultJson = cJSON_GetObjectItem(pRoot, "result");
    if (resultJson == NULL) {
        logger->error(LTRACE, "ChinaTelecomTools applyLoginFactor fail:%s", strResponse.c_str());
        cJSON_Delete(pRoot);
        return false;
    }

    outPukKey = cJSON_GetObjectItem(resultJson, "nonce")->valuestring;
    cJSON_Delete(pRoot);

    return true;
}

int ChinaTelecomTools::getPhoneOperator(const string& phone)
{
    string pubKey;
    if (!applyLoginFactor(phone, pubKey)) {
        logger->error(LTRACE, "ChinaTelecomTools getPubKey fail ....");
        return OPERATOR_TYPE_UNKNOW;
    }
    
    string phoneAttrVar;
    if (!queryPhoneAttribution(phone, pubKey, phoneAttrVar)) {
        logger->error(LTRACE, "ChinaTelecomTools getPhoneOperation:%s fail ....", phone.c_str());
        return OPERATOR_TYPE_UNKNOW;
    }
    
    return doParse(phone, phoneAttrVar);
}

bool ChinaTelecomTools::queryPhoneAttribution(const string& phone, const string& publicKey, string& outRaw)
{
    // {
    //     "data": "eTt3ZAyN8S/Odl+C37w4hOzSvcDWhCgofvBXGySjOPDtxQYDAdI24N+Q20Mt/jRhWSSuW2oIuh0mWUNubtbKfdbsmfZvU00+i+f8WCyCh/cHY2CK9NFjgmDIfjzVs10MPC//Rgw5oZnmNjBsR6n0d8zWZ1NINicPcRuaaqKQR8FmwfoeN469SZ1+exZTRPNEDCAUR8niFkXeDoika05LdnZ5nQf4jrmE+IyyUHcdTM8zHlPr6BMQkO+fGbXiu3xRQQIWMTWGtR2aYMV0CF4kDFSMgeHAwggIpi5jgn9x/cXHLJMBNrdWNQl8q0WxB0jOJ2tPjr1PVDQ+guKyxCQNukQG4RVuyqILfp2ao9wNXWXKC2cyIrqlIZOPmC17wEzusY6gWM3Kr77O4g0rIz+9dcw792bG+wdpXVQkBmgbvdGU/SlH90fLdIvbhpXeIOOezQwHHoylr8cdJEF+zPDxS/UKSBL5IsgqNrvceK7JR8k=", 
    //     "key": "FuxToznxjyctLkX7yi6H/85YsDyFWUPSiVd2NwEb96Bx/9hIKq9I1DukybOt8RozFEM9AK03Xl1B7xNSxmkJfEqAGSuRqHEiXpZJODfisWjJeb3a4SGzpiS07vYdaoaJUsaQEBeU3lYh7uBWhWEh1KEnBdTp44EimtqQ8gi2zM4=", 
    //     "sign": "DD3DC109E06ACDC9777745616E5F9037", 
    //     "productNo": "17359001573", 
    //     "encyType": "C005"
    // }
    
    string embededPubKey("-----BEGIN PUBLIC KEY-----\n");
    embededPubKey.append(publicKey);
    embededPubKey.append("\n-----END PUBLIC KEY-----");

    char oriBuff[256] = { 0 };
    snprintf(oriBuff, 255, ORIGINAL_FMT, AGREE_ID, phone.c_str());
    string orginalStr(oriBuff);

    string mixedKey = generateMixed();
    string key = ZZ_TOOLS::RSA_TOOL::rsa_pub_encrypt(mixedKey, embededPubKey);
    string data = ZZ_TOOLS::AES_TOOL::encrypt_cbc(orginalStr, mixedKey, "");
    
    ZZTools::MD5 md5;
    string sign(md5.digestString(orginalStr.c_str()));
    transform(sign.begin(), sign.end(), sign.begin(), ::toupper);

    cJSON* reqData = cJSON_CreateObject();
    cJSON_AddItemToObject(reqData, "data", cJSON_CreateString(data.c_str()));
    cJSON_AddItemToObject(reqData, "key", cJSON_CreateString(key.c_str()));
    cJSON_AddItemToObject(reqData, "sign", cJSON_CreateString(sign.c_str()));
    cJSON_AddItemToObject(reqData, "productNo", cJSON_CreateString(phone.c_str()));
    cJSON_AddItemToObject(reqData, "encyType", cJSON_CreateString("C005"));

    char* buff = cJSON_PrintUnformatted(reqData);
    string postData(buff);
    cJSON_free(buff);
    cJSON_Delete(reqData);

    map<string, string> headers;
    headers.insert(make_pair<>("Content-Type", "application/json;charset=utf-8"));
    
    bool res = false;
    HttpClient httpClient;
    for (int i = 0; i < VACANT_MAX_RETRY; i++) {
        string strResponse;
        if (!(res = httpClient.PostSSL(DX_YZF_URL, headers, postData, NULL, VACANT_CONN_TIMEOUT, VACANT_READ_TIMEOUT, strResponse))) {
            logger->error(LTRACE, "ChinaTelecomTools doPost error");
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }
        
        // {"RSP_HEAD":{"RESPONSECODE":"PA9999","RESPONSECONTENT":"系统繁忙，请稍候再试"}}
        // {"result":null,"success":false,"errorCode":"API010B008","errorMsg":"H5解密错误"}
        // {"result":null,"success":false,"errorCode":"API010P001","errorMsg":"解密类型不能为空"}
        // {"result":null,"success":false,"errorCode":"API403","errorMsg":"签名验证失败"}
        // {"result":null,"success":false,"errorCode":"BZ00993","errorMsg":"号码属性查询失败"} 未分配的空号

        if (strResponse.empty() || strResponse.find("系统繁忙，请稍候再试") != string::npos) {
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }
        
        if (strResponse.find("BZ00993") != string::npos) {
            logger->error(LTRACE, "ChinaTelecomTools queryPhoneAttribution: %s, 号码属性查询失败, 未分配的号码", phone.c_str());
            break;
        }
        
        if (strResponse.find("API010B008") != string::npos || strResponse.find("API010P001") != string::npos
                || strResponse.find("API403") != string::npos) {
            break;
        }
        
        if (res) {
            outRaw = strResponse;
            break;
        }
    }
    
    return res;
}

int ChinaTelecomTools::doParse(const string& phone, const string& raw)
{
    if (raw == "") {
        return OPERATOR_TYPE_UNKNOW;
    }
        
    // {"result":{"areaCode":"010","cityName":"北京","carrierName":"lt","cityCode":"110100","proviceCode":"110000"},"success":true,"errorCode":null,"errorMsg":null}

    cJSON* pRoot = cJSON_Parse(raw.c_str());
    if (!cJSON_IsTrue(cJSON_GetObjectItem(pRoot, "success"))) {
        cJSON_Delete(pRoot);
        return OPERATOR_TYPE_UNKNOW;
    }
    
    cJSON* resultJson = cJSON_GetObjectItem(pRoot, "result");
    if (resultJson == NULL) {
        cJSON_Delete(pRoot);
        return OPERATOR_TYPE_UNKNOW;
    }
    
    string carrierName = cJSON_GetObjectItem(resultJson, "carrierName")->valuestring; // result.getString("carrierName");
    cJSON_Delete(pRoot);
    
    return parseOperatorToCode(carrierName);
}

int ChinaTelecomTools::parseOperatorToCode(const string& carrierName)
{
    int result = OPERATOR_TYPE_UNKNOW;
    if (carrierName == API_OPERATOR_YD) {
        result = OPERATOR_TYPE_CMCC;
    } else if (carrierName == API_OPERATOR_LT) {
        result = OPERATOR_TYPE_UNICOM;
    } else if (carrierName == API_OPERATOR_DX) {
        result = OPERATOR_TYPE_TELECOM;
    }
    return result;
}

string ChinaTelecomTools::generateMixed()
{
    string result("");
    for (int i = 0; i < 16; i++) {
        char temp[2] = { 0 };
        sprintf(temp, "%d", rand() % 10);
        result.append(temp);
    }
    return result;
}

}
