#include "safer_times.h"

namespace zz_tools
{

std::string put_time(const tm *date_time, const char *c_time_format)
{
    const size_t size = 1024;
    char buffer[size];
    auto success = strftime(buffer, size, c_time_format, date_time);

    if (0 == success)
    {
        return c_time_format;
    }

    return buffer;
}

//tm localtime(const std::time_t &time)
//{
//    std::tm tm_snapshot;
//#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
//    localtime_s(&tm_snapshot, &time);
//#else
//    localtime_r(&time, &tm_snapshot); // POSIX
//#endif
//    return tm_snapshot;
//}

tm gmtime(const std::time_t &time)
{
    std::tm tm_snapshot;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
    gmtime_s(&tm_snapshot, &time);
#else
    gmtime_r(&time, &tm_snapshot); // POSIX
#endif
    return tm_snapshot;
}

} //namespace zz_tools
