#include <lwpr.h>

#include "ChinaUnicomTools.h"
#include "LarkBotUtils.h"
#include "HttpClient.h"
#include "GlobalRes.h"
#include "CollectFixDef.h"
#include "Cipher.h"

using namespace ZZTools;

namespace ZZCollect
{

extern GlobalRes GLOBAL_RES;

int ChinaUnicomTools::checkUnicomPhoneVacant(const string& phone)
{
    // "https://upay.10010.com/npfweb/NpfWeb/Mustpayment/getMustpayment?number=%s&province=%s&commonBean.phoneNo=%s&channelType=%s&GET"
    // province参数后端没有做校验,可以写死 "011", CHANNEL_TYPE = "101"
    char buff[256] = { 0 };
    snprintf(buff, 255, URL_MUST_PAYMENT, phone.c_str(), "011", phone.c_str(), "101");
    string url(buff);
    
    int result = PHONE_STATUS_DEFAULT;
    HttpClient httpClient;
    
    for (int i = 0; i < VACANT_MAX_RETRY; i++) {
        string strResponse;
        if (!httpClient.GetSSL(url, NULL, VACANT_CONN_TIMEOUT, VACANT_READ_TIMEOUT, strResponse)) {
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }

        // 正常 {"feeinfo":"0","rspCode":"0000","lastPay":"","receiptFlag":false,"receiptTwoFlag":false,"notation":"notation","mustPay":"0","payPlatform":"unipay","out":"success"}
        //      {"feeinfo":"0","rspCode":"2210","lastPay":"","notation":"notation","mustPay":"0","payPlatform":"unipay","out":"您是合账用户，请前往营业厅交费。[2210]"}
        // 注销 {"feeinfo":"0","rspCode":"1612","lastPay":"","notation":"notation","mustPay":"0","payPlatform":"unipay","out":"请到“服务-交费-销户号码交费”进行操作，若仍有异常可咨询10010。[1612]"}
        // 空号 {"feeinfo":"0","rspCode":"H888","lastPay":"","notation":"notation","mustPay":"0","payPlatform":"unipay","out":"您好，您的信息资料不全，详情可咨询10010或当地营业厅。[H888]"}
        // 服务流控 {"out":"busy"}
        if (strResponse.find(BUSY_INFO) != string::npos) {
            logger->error(LTRACE, "unicom getMustpayment busy ....");
            // 重试可能还是流控更加导致当前IP不可用
            result = CHK_BUSY;
            // notifyBusyAlarmMsg();
            break;
        }

        if (strResponse.empty()) {
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }

        logger->info(LTRACE, "Unicom %s getMustpayment:%s", phone.c_str(), strResponse.c_str());

        cJSON* pRoot = cJSON_Parse(strResponse.c_str());
        cJSON* pRspCodeItem = cJSON_GetObjectItem(pRoot, "rspCode");
        if (pRspCodeItem == NULL) {
            cJSON_Delete(pRoot);
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }

        string rspCode = pRspCodeItem->valuestring;
        cJSON_Delete(pRoot);

        if (rspCode == "0000" || rspCode == "2210") {
            result = PHONE_STATUS_VALID;
        } else if (rspCode == "1612") {
            result = PHONE_STATUS_SHUTDOWN_NUM;
        } else if (rspCode == "H888") {
            result = PHONE_STATUS_VACANT_NUM;
        }
        
        break;
    }

    return result;
}

bool ChinaUnicomTools::notifyBusyAlarmMsg() {
    string larkAlarmUrl = GLOBAL_RES.globalConf.commonConf.larkAlarmUrl;
    string collectorId = GLOBAL_RES.globalConf.commonConf.collectorId;
    
    char buff[256] = { 0 };
    snprintf(buff, 255, "%s request unicom busy, collectorId:%s", ZZTools::Utility::DateTimeNow().c_str(), collectorId.c_str());
    string msg(buff);
    
    bool ret = LarkBotUtils::sendLarkTextMsg(larkAlarmUrl, msg);
    if (ret) {
        logger->info(LTRACE, "lark alarm send ok, %s", msg.c_str());
    } else {
        logger->error(LTRACE, "lark alarm send nok, %s", msg.c_str());
    }

    return ret;
}

int ChinaUnicomTools::getPhoneOperator(const string& phone)
{
    // "https://upay.10010.com/npfweb/NpfWeb/customInfo/cellInfoQuery?commonBean.phoneNo=%s&loginPhoneNo=&commonBean.channelType=101"
    char buff[256] = { 0 };
    snprintf(buff, 255, URL_CELL_INFO, phone.c_str());
    string url(buff);
    
    int result = OPERATOR_TYPE_UNKNOW;
    HttpClient httpClient;
    
    for (int i = 0; i < VACANT_MAX_RETRY; i++) {
        string strResponse;
        if (!httpClient.GetSSL(url, NULL, VACANT_CONN_TIMEOUT, VACANT_READ_TIMEOUT, strResponse)) {
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }

        // {"isLogin":false,"carrier":"TEL","provinceCode":"038","cityCode":"380","custName":""}
        // {"isLogin":false,"carrier":"MOB","provinceCode":"074","cityCode":"791","custName":""}
        // {"isLogin":false,"carrier":"UNI","provinceCode":"084","cityCode":"844","is4g":true,"custName":""}
        // {"isLogin":false,"carrier":"TEL","provinceCode":"084","cityCode":"841","custName":""}
        // {"isLogin":false,"custName":""}
        // {"out":"busy"}
        if (strResponse.find(BUSY_INFO) != string::npos) {
            logger->error(LTRACE, "unicom getMustpayment busy ....");
            // 重试可能还是流控更加导致当前IP不可用
            result = OPERATOR_TYPE_BUSY;
            sleep(INTERVAL_WHEN_ERR);
            // notifyBusyAlarmMsg();
            break;
        }

        if (strResponse.empty()) {
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }

        logger->info(LTRACE, "Unicom %s cellInfoQuery:%s", phone.c_str(), strResponse.c_str());

        cJSON* pRoot = cJSON_Parse(strResponse.c_str());
        cJSON* pRspCarrierItem = cJSON_GetObjectItem(pRoot, "carrier");
        if (pRspCarrierItem == NULL) {
            cJSON_Delete(pRoot);
            sleep(INTERVAL_WHEN_ERR);
            continue;
        }

        string rspCarrier = pRspCarrierItem->valuestring;
        if (rspCarrier == "TEL") {
            result = OPERATOR_TYPE_TELECOM;
        } else if (rspCarrier == "MOB") {
            result = OPERATOR_TYPE_CMCC;
        } else if (rspCarrier == "UNI") {
            result = OPERATOR_TYPE_UNICOM;
        }
        
        cJSON_Delete(pRoot);
        break;
    }

    return result;
}

}
