#include "GlobalRes.h"

namespace ZZCollect
{

bool GlobalRes::initConf()
{
    return readConfig("./etc/zz_collect.toml", this->globalConf);
}

}