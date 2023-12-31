#pragma once

#include <zz_server.h>
#include "cacheable_response.h"
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <chrono>


// NOTE: Apple prior to macOS 12 doesn't support shared mutexes :(
// This is a ridiculous hack.
#if defined (__APPLE__)
#include <Availability.h>
#if __apple_build_version__ < 8020000
#pragma message ( "No support for std::shared_lock!" )
#define NO_SHARED_LOCK
#endif
#endif

#if defined(NO_SHARED_LOCK)
#define SHARED_LOCK std::unique_lock
#define SHARED_MUTEX std::mutex
#else
#define SHARED_LOCK std::shared_lock
#define SHARED_MUTEX std::shared_timed_mutex
#endif

namespace zz_tools
{

class response_renderer
{
public:
    response_renderer();

    std::shared_ptr<cacheable_response> render(const zz_tools::request &request, zz_tools::response &response);

    // option setters
    void set_option(const server::server_identifier &value);
    void set_option(const server::server_identifier_and_version &value);
    void set_option(const server::append_to_server_identifier &value); //TODO I am not fond of having this here.
    void set_option(server::enable_internal_file_cache value);
    void set_option(server::internal_file_cache_keep_alive value);
    void set_option(server::not_found_handler_cb value);

private:
    std::shared_ptr<cacheable_response> from_file_(const zz_tools::request &request, zz_tools::response &response);

    std::string server_identifier_;

    // fd cache
    bool use_fd_cache_;
    static std::mutex fd_mutex_;
    static SHARED_MUTEX fd_cache_mutex_;
    std::unordered_map<std::string, std::shared_ptr<cacheable_response> > fd_cache_;
    std::chrono::milliseconds cache_keep_alive_;

    // custom user-supplied 404 renderer
    server::not_found_handler_cb not_found_handler_;
};



} //namespace zz_tools
