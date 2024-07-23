#pragma once

#include <string>

using namespace std;

namespace ZZCollect
{

#define HEADER_RET_CODE               "RET_CODE"
#define HEADER_RET_INFO               "RET_INFO"

#define CMCC_AES_KEY                  "043AOQGK6ykklyZA"
#define CMCC_AES_IV                   "043AOQGK6ykklyZA"

#define RET_CODE_OK                    0
#define RET_CODE_NOK                  -1

#define OPERATOR_TYPE_BUSY            -2
#define OPERATOR_TYPE_UNKNOW          -1
#define OPERATOR_TYPE_UNICOM           0
#define OPERATOR_TYPE_TELECOM          1
#define OPERATOR_TYPE_CMCC             2

#define SLEEP_WHEN_NO_DATA             5
#define VACANT_MAX_RETRY               5
#define VACANT_RETRY_INTERVAL          3
#define INTERVAL_WHEN_ERR              15
#define SLEEP_WHEN_CK_EXPIRED          15
#define VACANT_CONN_TIMEOUT            5
#define VACANT_READ_TIMEOUT            5
#define FETCH_INTERVAL                 15

#define VACANT_ALARM_TYPE_MOBILE       1
#define VACANT_ALARM_TYPE_UNICOM       2
#define VACANT_ALARM_TYPE_TELECOM      3

#define PHONE_STATUS_DEFAULT           0          //  0.初始状态待爬取
#define PHONE_STATUS_VACANT_NUM       -1          // -1.空号
#define PHONE_STATUS_SHUTDOWN_NUM     -2          // -2.注销
#define PHONE_MOVED                   -3          // -3.转网
#define PHONE_STATUS_VALID             1          //  1.正常号码
#define CHK_NEED_RETRY                 2          //  2.重试
#define CHK_LOGIN_EXPIRED              3          //  3.登录过期
#define CHK_OVERLOAD                   4          //  4.下单次数过多
#define CHK_BUSY                       5          //  5.流控
#define CHK_SERV_INVALID               6          //  6.后端服务暂停

#define ALARM_TTL                      150000

#define PHONE_STATUS_LOGIN_EXP        "500004"    // 
#define PHONE_STATUS_MOVED            "500005"    // 非移动号码
#define PHONE_OVERLOAD                "530002"    // 下单登录检测次数过多
#define PHONE_STATUS_OK               "532014"    // 正常号码
#define PHONE_STATUS_OK2              "531014"    // 号码正常(它机下单)
#define PHONE_STATUS_OK3              "532011"    // 该省份无法使用该优惠
#define PHONE_STATUS_NOK              "531005"    // 用户状态异常
#define PHONE_STATUS_CANCEL           "531011"    // 用户已经销户
#define PHONE_STATUS_NONEXISTENCE     "531012"    // 此号码不存在
#define PHONE_CHARGE_NOT_IN_SERVICE   "531002"    // 11点后不让缴费
#define PHONE_STATUS_531008           "531008"    // 该号码不支持充值

#define LOGIN_PARAMS                  "productNo=%s&requestType=H5&callback=&appType=03&appVersion=10.66.60&timestamp=%ld"
#define URL_LOGIN_FACTOR              "https://mapi-h5.bestpay.com.cn/gapi/mapi-gateway/applyLoginFactor"
#define AGREE_ID                      "20210525030100136633395265404954"
#define DX_YZF_URL                    "https://mapi-h5.bestpay.com.cn/gapi/handyservice/communication/queryPhoneAttribution"

#define URL_CELL_INFO                 "https://upay.10010.com/npfweb/NpfWeb/customInfo/cellInfoQuery?commonBean.phoneNo=%s&loginPhoneNo=&commonBean.channelType=101"
#define URL_MUST_PAYMENT              "https://upay.10010.com/npfweb/NpfWeb/Mustpayment/getMustpayment?number=%s&province=%s&commonBean.phoneNo=%s&channelType=%s&GET"

#define API_OPERATOR_YD               "yd"
#define API_OPERATOR_LT               "lt"
#define API_OPERATOR_DX               "dx"

#define BUSY_INFO                     "{\"out\":\"busy\"}"
#define ORIGINAL_FMT                  "{\"agreeId\":\"%s\",\"mobileNo\":\"%s\"}"

class VacantTask
{
public:
    int    oper;
    string prov;
    string ck;
    string phoneList;
};

class VacantResult
{
public:
    VacantResult(const string& strPhone, int iStatus)
        : phone(strPhone), status(iStatus)
    {

    }
    
    ~VacantResult()
    {
        
    }

public:
    string phone;
    int    status;
};

class MigrateResult
{
public:
    MigrateResult(const string& strPhone, int iOper)
        : phone(strPhone), oper(iOper)
    {

    }

    ~MigrateResult() {  }

private:
    string phone;
    int    oper;
};

}
