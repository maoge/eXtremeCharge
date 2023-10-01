#pragma once

#include "CollectConfig.h"

namespace ZZCollect
{

class GlobalRes
{
public:
    bool initConf();

public:
    GlobalConf globalConf;
    string     prov = "吉林";
    bool       vacantEnabled = false;

};

}
