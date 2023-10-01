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

#define OPERATOR_UNICOM                0
#define OPERATOR_TELECOM               1
#define OPERATOR_CMCC                  2

#define SLEEP_WHEN_NO_DATA             5
#define VACANT_MAX_RETRY               5
#define VACANT_RETRY_INTERVAL          1
#define SLEEP_WHEN_CK_EXPIRED          15
#define VACANT_CONN_TIMEOUT            5
#define VACANT_READ_TIMEOUT            5

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

}
