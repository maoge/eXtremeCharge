#include <iostream>
#include <cpptoml.h>

#include <stdexcept>
#include <limits>

#include "lwpr.h"
#include "CollectConfig.h"

namespace ZZCollect {

CollectConf::CollectConf() 
    : vacantMobileNumberWorkerPoolCoreSize(7), 
      vacantMobileNumberWorkerPoolMaxSize(7),
      vacantMobileSwitch(true)
{
	
}

bool readConfig(const char *file, GlobalConf& globalConf)
{
    std::shared_ptr<cpptoml::table> parser;
    try {
        logger->info(LTRACE, "parse config file: %s", file);
        parser = cpptoml::parse_file(file);
    } catch (const cpptoml::parse_exception &e) {
        logger->error(LTRACE, "parse config file error: %s", e.what());
        return false;
    }

    auto Strings = [&](const char *key, const char * value)->std::string {
        if (parser->contains_qualified(key)) {
            return parser->get_qualified(key)->as<std::string>()->get();
        }
        if (!value)
            return std::string();
        return std::string(value);
    };

    auto Integer = [&](const char *key, int value)->int {
        if (parser->contains_qualified(key)) {
            auto integer = parser->get_qualified(key)->as<int64_t>()->get();
            return static_cast<int>(integer);
        }
        return value;
    };

    auto Boolean = [&](const char *key, bool value)->bool {
        if (parser->contains_qualified(key)) {
            auto b = parser->get_qualified(key)->as<bool>()->get();
            return static_cast<bool>(b);
        }
        return value;
    };

    globalConf.collectConf.vacantMobileNumberWorkerPoolCoreSize = Integer("collect.vacant_mobile_number_worker_pool_core_size", 7);
    globalConf.collectConf.vacantMobileNumberWorkerPoolMaxSize  = Integer("collect.vacant_mobile_number_worker_pool_max_size", 7);
    globalConf.collectConf.vacantMobileSwitch                   = Boolean("collect.vacant_mobile_switch", false);

    globalConf.commonConf.taskCenterAddress                     = Strings("common.task_center_address", "");
    globalConf.commonConf.pushCenterAddress                     = Strings("common.push_center_address", "");
    globalConf.commonConf.resId                                 = Integer("common.res_id", 1);
    globalConf.commonConf.collectorId                           = Strings("common.collector_id", "127.0.0.1");
    globalConf.commonConf.larkAlarmUrl                          = Strings("common.lark_alarm_url", "");

    return true;
}

}
