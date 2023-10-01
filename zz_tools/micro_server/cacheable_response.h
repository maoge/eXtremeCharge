#pragma once

#include <types.h>
#include <microhttpd.h>
#include <unordered_map>
#include <chrono>

namespace zz_tools
{

struct cacheable_response
{
    struct MHD_Response *mhd_response;
    zz_tools::status_code status_code;

    bool cached;
    std::chrono::system_clock::time_point time_cached;

    cacheable_response(struct MHD_Response *mhd_response, zz_tools::status_code status_code);

    ~cacheable_response();
};

} //namespace zz_tools
