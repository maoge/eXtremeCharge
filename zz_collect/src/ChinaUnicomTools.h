#pragma once

using namespace std;

namespace ZZCollect
{

class ChinaUnicomTools {

public:
    static int checkUnicomPhoneVacant(const string& phone);
    static int getPhoneOperator(const string& phone);

private:
    static bool notifyBusyAlarmMsg();

};

}
