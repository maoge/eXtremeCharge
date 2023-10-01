#pragma once

#include <router.h>

namespace zz_tools
{

class service_framework
{
public:
    service_framework(std::shared_ptr<router>& router);
    virtual ~service_framework();

    virtual void register_service() = 0;
    void add_handler(zz_tools::request_method method,
                     std::__cxx11::string route,
                     zz_tools::router::endpoint_handler_cb callback,
                     zz_tools::parameter::validators validations = {})
    {
        router_->handle_request(method, route, callback, validations);
    }

private:
    std::shared_ptr<router>& router_;

};

} //namespace zz_tools
