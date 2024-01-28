#include <nlohmann/json.hpp>
#include <iostream>

#include "Base64.h"
#include "MobileVacantProber.h"
#include "VacantProberTool.h"
#include "GlobalRes.h"
#include "HttpClient.h"
#include "Cipher.h"

using namespace ZZTools;

namespace ZZCollect
{

extern GlobalRes GLOBAL_RES;

static const char* IN_PARAM_SAVE_ORDER = "{\"channel\":\"0003\",\"payWay\":\"WAP\",\"amount\":%.02f,\"chargeMoney\":%d,\"choseMoney\":%d,\"activityNO\":\"\",\"operateId\":%d,\"homeProv\":\"%d\",\"numFlag\":\"0\",\"source\":\"\",\"invoinceType\":\"\"}";
static const char* URL_CHECK_PHONE = "https://touch.10086.cn/i/v1/pay/saveorder4t/%s?provinceId=%d";
static const char* PUSH_URL_FMT = "http://%s/api/collect/notifyVacantNumTask";

MobileVacantProber::MobileVacantProber(const string& sTaskCenterAddr, const string& sPushCenterAddr, const string& sProberName)
    : m_sTaskCenterAddr(sTaskCenterAddr), m_sPushCenterAddr(sPushCenterAddr), m_sProberName(sProberName)
{
    
}

MobileVacantProber::~MobileVacantProber()
{
    
}

void MobileVacantProber::Run()
{
    int resId = GLOBAL_RES.globalConf.commonConf.resId;
    while (IsContinue()) {
        logger->debug(LTRACE, "%s running ....", m_sProberName.c_str());

        if (!GLOBAL_RES.globalConf.collectConf.vacantMobileSwitch)
        {
            logger->info(LTRACE, "vacant mobile disabled ....");
            sleep(SLEEP_WHEN_NO_DATA);
            continue;
        }

        VacantTask vacantTask;
        if (!VacantProberTool::fetchVacantNumTask(m_sTaskCenterAddr, GLOBAL_RES.prov, OPERATOR_TYPE_CMCC, resId, vacantTask))
        {
            logger->info(LTRACE, "vacant mobile fetch with no data ....");
            sleep(SLEEP_WHEN_NO_DATA);
            continue;
        }

        vector<VacantResult> result;
        checkVacantPhoneTask(vacantTask, result);

        doNotify(result);
    }
}

void MobileVacantProber::doNotify(const vector<VacantResult>& vacantResultVec)
{
    cJSON* pRoot = cJSON_CreateObject();
    cJSON* pDataArr = cJSON_CreateArray();

    vector<VacantResult>::const_iterator it = vacantResultVec.begin();
    vector<VacantResult>::const_iterator end = vacantResultVec.end();
    for (; it != end; it++)
    {
        const VacantResult& vacantResult = *it;
        cJSON* item = cJSON_CreateObject();
        cJSON_AddItemToObject(item, "phone", cJSON_CreateString(vacantResult.phone.c_str()));
        cJSON_AddItemToObject(item, "status", cJSON_CreateNumber(vacantResult.status));

        cJSON_AddItemToArray(pDataArr, item);
    }

    cJSON_AddItemToObject(pRoot, "data", pDataArr);
    char* buff = cJSON_PrintUnformatted(pRoot);
    string notifyData(buff);

    cJSON_free(buff);
    cJSON_Delete(pRoot);

    static string PUSH_URL = "http://" + GLOBAL_RES.globalConf.commonConf.pushCenterAddress + "/api/collect/notifyVacantNumTask";
    HttpClient httpClient;

    for (int i = 0; i < VACANT_MAX_RETRY; i++)
    {
        string strResponse;
        if (httpClient.Post(PUSH_URL, notifyData, 5, 5, strResponse))
        {
            logger->debug(LTRACE, "doNotify OK, url: %s, notifyData: %s", PUSH_URL.c_str(), notifyData.c_str());
            break;
        }
        else
        {
            logger->error(LTRACE, "doNotify NOK, url: %s, notifyData: %s, retry ....", PUSH_URL.c_str(), notifyData.c_str());
            sleep(VACANT_RETRY_INTERVAL);
        }
    }
}

void MobileVacantProber::checkVacantPhoneTask(const VacantTask& vacantTask, vector<VacantResult>& result)
{
    int oper = vacantTask.oper;
    string ck = vacantTask.ck;
    string prov = vacantTask.prov;
    string phoneList = vacantTask.phoneList;

    vector<string> phoneVec;
    StringUtil::SplitString(phoneList, ',', phoneVec);

    vector<string>::iterator it = phoneVec.begin();
    vector<string>::iterator end = phoneVec.end();
    for (; it != end; it++)
    {
        string phone = *it;
        int status = checkVacantPhone(phone, prov, ck);
        if (status == CHK_LOGIN_EXPIRED || status == CHK_OVERLOAD)
        {
            static const char* ALARM_FMT = "%s 资源组resID:%d %s, ck:%s";
            char buff[256] = {"\0"};
            int resId = GLOBAL_RES.globalConf.commonConf.resId;
            snprintf(buff, 255, ALARM_FMT, Utility::DateTimeNow().c_str(), resId, 
                    (status == CHK_LOGIN_EXPIRED ? "辅助token过期" : "登录号码下单次数过多"), ck.c_str());
            string alarmInfo(buff);

            VacantProberTool::notifyLarkAlarm(VACANT_ALARM_TYPE_MOBILE, alarmInfo);
            VacantProberTool::notifyCheckCKInvalid(GLOBAL_RES.globalConf.commonConf.resId);
                    
            status = PHONE_STATUS_DEFAULT;
            sleep(SLEEP_WHEN_CK_EXPIRED);
        }
                
        VacantResult checkResult(phone, status);
        result.push_back(checkResult);
    }
}

int MobileVacantProber::checkVacantPhone(const string& phone, const string& prov, const string& ck)
{
    static const std::string AES_KEY(CMCC_AES_KEY);
	static const std::string AES_IV(CMCC_AES_IV);

    int provId = getProvId(prov);
    string payPhoneNo = phone.substr(0, 8);

    string phoneEncData = ZZ_TOOLS::AES_TOOL::encrypt_cbc(phone, AES_KEY, AES_IV);
    string base64PhoneData = base64_encode(phoneEncData);

	int payRuleId = getPayRuleId(prov);
    int chargeMoney = 10;
    double amount = 9.98;

	// 1.查询pay rule
	// pay rule只有固定的两种,不用耗时查询后台

    map<string, string> reqHeaderSaveOrder;
    reqHeaderSaveOrder.insert(pair<string, string>("Connection", "keep-alive"));
    reqHeaderSaveOrder.insert(pair<string, string>("Sec-Ch-Ua-Mobile", "?0"));
    reqHeaderSaveOrder.insert(pair<string, string>("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36"));
    reqHeaderSaveOrder.insert(pair<string, string>("Content-Type", "application/json; charset=UTF-8"));
    reqHeaderSaveOrder.insert(pair<string, string>("Accept", "application/json, text/javascript, */*; q=0.01"));
    reqHeaderSaveOrder.insert(pair<string, string>("X-Requested-With", "XMLHttpRequest"));
    reqHeaderSaveOrder.insert(pair<string, string>("Origin", "https://touch.10086.cn"));
    reqHeaderSaveOrder.insert(pair<string, string>("Sec-Fetch-Site", "same-origin"));
    reqHeaderSaveOrder.insert(pair<string, string>("Sec-Fetch-Mode", "cors"));
    reqHeaderSaveOrder.insert(pair<string, string>("Sec-Fetch-Dest", "empty"));
    reqHeaderSaveOrder.insert(pair<string, string>("Referer", "https://touch.10086.cn/i/mobile/rechargecredit.html?welcome=" + std::to_string(ZZTools::Utility::GetMillisecondsFromLocal())));
    reqHeaderSaveOrder.insert(pair<string, string>("Accept-Language", "zh-CN,zh-HK;q=0.9,zh;q=0.8,zh-TW;q=0.7"));
	reqHeaderSaveOrder.insert(pair<string, string>("PayPhoneNo", payPhoneNo));
    reqHeaderSaveOrder.insert(pair<string, string>("Cookie", ck));

    char buff[256] = {"\0"};
    snprintf(buff, 255, IN_PARAM_SAVE_ORDER, amount, chargeMoney, chargeMoney, payRuleId, provId);
    string inParamSaveOrder(buff);

    string aesDecodeInParamSaveOrder = ZZ_TOOLS::AES_TOOL::encrypt_cbc(inParamSaveOrder, AES_KEY, AES_IV);

    string saveOrderData = "{\"inParam\":\"" + base64_encode(aesDecodeInParamSaveOrder) + "\"}";
    
    char urlBuff[512] = {"\0"};
	snprintf(urlBuff, 511, URL_CHECK_PHONE, base64PhoneData.c_str(), provId);
	string urlOrder(urlBuff);

    int result = PHONE_STATUS_DEFAULT;
    HttpClient httpClient;
	// 并发请求时可能网络超时出错, 需要重试
    for (int i = 0; i < VACANT_MAX_RETRY; i++)
    {
        string strResponse;
        if (!httpClient.PostSSL(urlOrder, reqHeaderSaveOrder, saveOrderData, NULL, VACANT_CONN_TIMEOUT, VACANT_READ_TIMEOUT, strResponse))
        {
            logger->info(LTRACE, "%s post fail ....", phone.c_str());
            sleep(VACANT_RETRY_INTERVAL);
            continue;
        }

        // 目前测出来就只有以下几种结果
        /**
         * {"retCode":"500002","retMsg":"参数格式非法","requestNo":"20230802130323503236074","data":null}
         * {"retCode":"500004","retMsg":"非登录状态","requestNo":"20230207131930460235364","data":null}
         * 
         * {"retCode":"532014","retMsg":"该优惠只可登录后本机充值享受","requestNo":"20230114025926727597540","data":null}
         * {"retCode":"532011","retMsg":"该省份无法使用该优惠","requestNo":"20230908233607058308314","data":null}
         * {"retCode":"531014","retMsg":"他机下单接口提示","requestNo":"20230207112103688394306","data":null}
         * 
         * {"retCode":"500005","retMsg":"非移动号码","requestNo":"20230207104731238237288","data":null}
         * {"retCode":"531005","retMsg":"用户状态异常","requestNo":"20230114025850208523867","data":null}
         * {"retCode":"531011","retMsg":"用户已经销户","requestNo":"20230114030131521226848","data":null}
         * {"retCode":"531012","retMsg":"此号码不存在","requestNo":"20230114025643977301533","data":null}
         * {"retCode":"531008","retMsg":"该号码不支持充值","requestNo":"20230908235118460591401","data":null}
         * 
         * {"retCode":"530002","retMsg":"登录号码下单次数过多","requestNo":"20230223192734718220031","data":null}
         * {"retCode":"531002","retMsg":"交费功能不可用","requestNo":"20230803233629045232705","data":null}
         * {"retCode":"532004","retMsg":"优惠规则不存在，请重新选择优惠信息","requestNo":"20230210122335400235654","data":null}
         * 
         */
        if (strResponse.empty())
        {
            logger->info(LTRACE, "save order response null");
            sleep(VACANT_RETRY_INTERVAL);
            return PHONE_STATUS_DEFAULT;
        }

        // logger->info(LTRACE, "%s vacant check response:%s", phone.c_str(), strResponse.c_str());
        
        // <!DOCTYPE html><html xmlns="http://www.w3.org/1999/xhtml">   
        //    <head>  <meta http-equiv="Content-Type" content="text/html; charset=utf-8"> 
        //            <title>error页面—中国移动网上商城</title> 
        //            <script>        (function (doc, win) {            var docEl = doc.documentElement;            var resizeEvt = 'orientationchange' in window ?                    'orientationchange' :                    'resize';            function recalc() {                var clientWidth = docEl.clientWidth;                if (!clientWidth)                    return;                if (clientWidth >= 750) {                    docEl.style.fontSize = '100px';                } else {                    docEl.style.fontSize = docEl.clientWidth / 7.5 + 'px';                }            };            if (!doc.addEventListener) return;            win.addEventListener(resizeEvt, recalc, false);            doc.addEventListener('DOMContentLoaded', recalc, false);            //当dom加载完成时，或者 屏幕垂直、水平方向有改变进行html的根元素计算        })(document, window);    </script>   <style type="text/css">     .error404{          height: 3.96rem;            /*background: url(../../pub-ui/images/bgs/netfail-5x.png) center center no-repeat;*/            background: url(https://shop.10086.cn/i/error/images/netfail-5x.png) center center no-repeat;           background-size: contain;           margin-top: 5.5rem;     }       .error404-back{         padding: 4rem 3.6rem 0;     }       .error404-back-home{            width: 2.4rem;          height: 0.85rem;            border: 1px solid #0085d0;          background: #f4f4f4;            border-radius: 0.08rem;         display: inline-block;          color: #0085d0;         text-align: center;         line-height: 0.8rem;            font-size: 0.35rem;         text-decoration: none;      }   </style>    </head> <body>      <div class="error404">          <div class="error404-back">             <a class="error404-back-home" href="" id="go-home">回到首页</a>         </div>      </div>  </body> <script type="text/javascript">     var url = window.location.href;     if(url.indexOf("/gray")>-1){            document.getElementById("go-home").setAttribute("href",window.location.protocol+"//touch.10086.cn/i/gray/mobile/home.html");        }else{          document.getElementById("go-home").setAttribute("href",window.location.protocol+"//touch.10086.cn/i/mobile/home.html");     }   </script></html>
        if (strResponse.find("<!DOCTYPE html>") != string::npos) {
            logger->info(LTRACE, "response html retry ....");
            sleep(VACANT_RETRY_INTERVAL);
            continue;
        }
        
        logger->info(LTRACE, "phone:%s, prov:%s, response:%s", phone.c_str(), prov.c_str(), strResponse.c_str());

        cJSON* pRoot = cJSON_Parse(strResponse.c_str());
        cJSON* pItem = cJSON_GetObjectItem(pRoot, "retCode");
        if (pItem == NULL) {
            cJSON_Delete(pRoot);
            return PHONE_STATUS_DEFAULT;
        }

        string retCode = pItem->valuestring;
        cJSON_Delete(pRoot);
        
        if (retCode.compare(PHONE_STATUS_OK) == 0 || retCode.compare(PHONE_STATUS_OK2) == 0 || retCode.compare(PHONE_STATUS_OK3) == 0)
        {
            result = PHONE_STATUS_VALID;
        }
        else if (retCode.compare(PHONE_STATUS_NOK) == 0 || retCode.compare(PHONE_STATUS_CANCEL) == 0)
        {
            result = PHONE_STATUS_SHUTDOWN_NUM;
        }
        else if (retCode.compare(PHONE_STATUS_NONEXISTENCE) == 0 || retCode.compare(PHONE_STATUS_531008) == 0)
        {
            result = PHONE_STATUS_VACANT_NUM;
        }
        else if (retCode.compare(PHONE_STATUS_MOVED) == 0)
        {
            result = PHONE_MOVED;
        }
        else if (retCode.compare(PHONE_STATUS_LOGIN_EXP) == 0)
        {
            result = CHK_LOGIN_EXPIRED;
            logger->error(LTRACE, "chkPhone:%s, 登录过期, resp_body:%s", phone.c_str(), strResponse.c_str());
        }
        else if (retCode.compare(PHONE_OVERLOAD) == 0)
        {
            result = CHK_OVERLOAD;
            logger->error(LTRACE, "chkPhone:%s, 登录号码下单次数过多", phone.c_str());
            sleep(SLEEP_WHEN_CK_EXPIRED);
        }
        else if (retCode.compare(PHONE_CHARGE_NOT_IN_SERVICE) == 0)
        {
            logger->error(LTRACE, "chkPhone:%s, 12点后移动缴费功能不可用", phone.c_str());
        }
        else
        {
            // logger->error(LTRACE, "chkPhone:%s, 状态:unknown", phone.c_str());
        }
        
        break;
    }

    return result;
}

int MobileVacantProber::getPayRuleId(const string& prov)
{
    if (prov == "河南" || prov == "广东")
    {
        return 11475;
    }
    return 11471;
}

map<string, int> MobileVacantProber::initProvMap()
{
    map<string, int> provIdMap;
    provIdMap.insert(pair<string, int>("北京", 100));
    provIdMap.insert(pair<string, int>("天津", 220));
    provIdMap.insert(pair<string, int>("河北", 311));
    provIdMap.insert(pair<string, int>("山西", 351));
    provIdMap.insert(pair<string, int>("内蒙古", 471));
    provIdMap.insert(pair<string, int>("辽宁", 240));
    provIdMap.insert(pair<string, int>("吉林", 431));
    provIdMap.insert(pair<string, int>("黑龙江", 451));
    provIdMap.insert(pair<string, int>("上海", 210));
    provIdMap.insert(pair<string, int>("江苏", 250));
    provIdMap.insert(pair<string, int>("浙江", 571));
    provIdMap.insert(pair<string, int>("安徽", 551));
    provIdMap.insert(pair<string, int>("福建", 591));
    provIdMap.insert(pair<string, int>("江西", 791));
    provIdMap.insert(pair<string, int>("山东", 531));
    provIdMap.insert(pair<string, int>("河南", 371));
    provIdMap.insert(pair<string, int>("湖北", 270));
    provIdMap.insert(pair<string, int>("湖南", 731));
    provIdMap.insert(pair<string, int>("广东", 200));
    provIdMap.insert(pair<string, int>("广西", 771));
    provIdMap.insert(pair<string, int>("海南", 898));
    provIdMap.insert(pair<string, int>("重庆", 230));
    provIdMap.insert(pair<string, int>("四川", 280));
    provIdMap.insert(pair<string, int>("贵州", 851));
    provIdMap.insert(pair<string, int>("云南", 871));
    provIdMap.insert(pair<string, int>("西藏", 891));
    provIdMap.insert(pair<string, int>("陕西", 290));
    provIdMap.insert(pair<string, int>("甘肃", 931));
    provIdMap.insert(pair<string, int>("青海", 971));
    provIdMap.insert(pair<string, int>("宁夏", 951));
    provIdMap.insert(pair<string, int>("新疆", 991));

    return provIdMap;
}

int MobileVacantProber::getProvId(const string& prov)
{
    static map<string, int> PROV_ID_MAP = initProvMap();

    int res = 100;
    map<string, int>::iterator it = PROV_ID_MAP.find(prov);
    if (it != PROV_ID_MAP.end())
    {
        res = it->second;
    }
    
    return res;
}

}
