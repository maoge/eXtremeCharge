#pragma once

#include <string>
#include <functional>
#include <types.h>

namespace zz_tools
{

enum class log_level
{
    FATAL = 0,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
};


using access_logger_cb = std::function<void(const request &request, const response &response)>;
using error_logger_cb = std::function<void(log_level level, const std::string &string)>;

void set_access_logger(access_logger_cb acc_logger);
void reset_access_logger();

void set_error_logger(error_logger_cb err_logger);
void reset_error_logger();

void access_log(const request& request, const response &response);
void error_log(log_level level, const std::string &string);

} //namespace zz_tools

std::string to_string(zz_tools::log_level value);
