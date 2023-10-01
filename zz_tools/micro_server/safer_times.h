#pragma once

#include <chrono>
#include <string>

namespace zz_tools
{

std::string put_time(const tm *date_time, const char *c_time_format);

//tm localtime(const time_t &time);

tm gmtime(const time_t &time);

}
