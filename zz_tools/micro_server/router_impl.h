#pragma once

#include <router.h>
#include <map>
#include <vector>
#include <tuple>
#include <mutex>

namespace zz_tools
{

class router::router_impl
{
public:
    router_impl(std::string route_base);


    void set_mime_type(std::string mime_type);

    using endpoint_handler_cb = std::function<response (const request &req)>;

    void handle_request(request_method method,
                        std::regex route,
                        endpoint_handler_cb callback,
                        parameter::validators validations = {});

    void handle_request(request_method method,
                        std::string route,
                        endpoint_handler_cb callback,
                        parameter::validators validations = {});

    void serve_files(std::string mount_point, std::string path_to_files);

    void add_header(std::string &&key, std::string &&value);

    OPT_NS::optional<zz_tools::response> process_request(request &request);

private:

    std::string route_base_;
    // std::mutex lock_;
    using request_handlers = std::vector<std::tuple<std::regex, endpoint_handler_cb, parameter::validators>>;
    std::map<request_method, request_handlers> request_handlers_;
    zz_tools::headers headers_;
    std::string mime_type_;
};

} //namespace zz_tools
