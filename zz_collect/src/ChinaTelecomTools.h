#pragma once

using namespace std;

namespace ZZCollect
{

class ChinaTelecomTools {

public:
    static bool applyLoginFactor(const string& phone, string& outPukKey);
    static int  getPhoneOperator(const string& phone);

private:
    static bool   queryPhoneAttribution(const string& phone, const string& publicKey, string& outRaw);
    static int    doParse(const string& phone, const string& raw);
    static int    parseOperatorToCode(const string& carrierName);
    static string generateMixed();

};

}
