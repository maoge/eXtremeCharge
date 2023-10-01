#include "config.h"
#include "types.h"
#include <sstream>
#include <regex>

std::string to_string(zz_tools::log_level value)
{
    switch (value)
    {
        case zz_tools::log_level::FATAL:
            return "FATAL";
        case zz_tools::log_level::ERROR:
            return "ERROR";
        case zz_tools::log_level::WARNING:
            return "WARNING";
        case zz_tools::log_level::INFO:
            return "INFO";
        case zz_tools::log_level::DEBUG:
        default:
            return "DEBUG";
    }
}

namespace zz_tools
{

access_logger_cb access_logger_ = nullptr;

void set_access_logger(access_logger_cb access_logger)
{
    access_logger_ = access_logger;
}

void reset_access_logger()
{
    access_logger_ = nullptr;
}

error_logger_cb error_logger_ = nullptr;

void set_error_logger(error_logger_cb error_logger)
{
    error_logger_ = error_logger;
}

void reset_error_logger()
{
    error_logger_ = nullptr;
}


void access_log(const request &request, const response &response)
{
    if(access_logger_)
        access_logger_(request, response);
}

void error_log(zz_tools::log_level level, const std::string &message)
{
    if(error_logger_)
        error_logger_(level, message);
}

} //namespace zz_tools
