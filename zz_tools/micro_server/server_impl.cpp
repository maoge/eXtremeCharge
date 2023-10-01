#include <arpa/inet.h>
#include <lwpr.h>
#include <nlohmann/json.hpp>
#include "server_impl.h"

namespace zz_tools
{

#define CONTENT_JSON    "application/json"
#define CONTENT_TEXT    "text/plain"

const server::accept_policy_cb default_accept_policy_callback_ = [](const struct sockaddr *addr,
                                                                    socklen_t len) -> bool
{
    return true;
};

server::server_impl::server_impl() :
        debug_output_{false},
        ssl_mem_cert_set_{false},
        ssl_mem_key_set_{false},
        use_thread_per_connection_{false},
        use_epoll_if_available_{false},
        daemon_{nullptr},
        accept_policy_callback_{default_accept_policy_callback_},
        port_{0},
        server_name_{ZZ_NAME}
{ }


//////// public functions

bool server::server_impl::start(uint16_t port)
{
    auto result = start_async(port);

    if (!result) return result;

    // TODO would be better to find a way to run MHD in _impl_ thread...
    await();

    return true;
}

bool server::server_impl::start_async(uint16_t port)
{
    port_ = port;

    MHD_OptionItem options[options_.size() + 1];
    uint16_t idx = 0;
    for (const auto &opt : options_)
    {
        options[idx++] = opt; //copy it in, whee.
    }
    options[idx] = {MHD_OPTION_END, 0, nullptr};

    unsigned int flags = MHD_NO_FLAG;

    if (debug_output_)
    {
        logger->info(LTRACE, "Enabling debug output");
        flags |= MHD_USE_DEBUG;
    }

    if (ssl_mem_cert_set_ && ssl_mem_key_set_)
    {
        logger->info(LTRACE, "Enabling SSL");
        flags |= MHD_USE_SSL;
    }
    else if (ssl_mem_cert_set_ || ssl_mem_key_set_)
    {
        logger->info(LTRACE, "Please provide both server::https_mem_key AND server::https_mem_cert");
        return false;
    }

    if (use_thread_per_connection_)
    {
        logger->info(LTRACE, "Will use one thread per connection");
        flags |= MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL;
    }
    else if (use_epoll_if_available_)
    {
#if defined(__linux__)
        logger->info(LTRACE, "Will use epoll");
        flags |= MHD_USE_EPOLL_INTERNALLY;
#else
        logger->info(LTRACE, "Will use poll");
        flags |= MHD_USE_POLL_INTERNALLY;
#endif
    }
    else
    {
        logger->info(LTRACE, "No threading options set, will use select");
        flags |= MHD_USE_SELECT_INTERNALLY;
    }

    daemon_ = MHD_start_daemon(flags,
                                      port_,
                                      access_policy_callback_shim_, this,
                                      access_handler_callback_shim_, this,
                                      MHD_OPTION_NOTIFY_COMPLETED, request_completed_callback_shim_, this,
                                      MHD_OPTION_EXTERNAL_LOGGER, logger_callback_shim_, nullptr,
                                      MHD_OPTION_URI_LOG_CALLBACK, uri_logger_callback_shim_, nullptr,
                                      MHD_OPTION_ARRAY, options,
                                      MHD_OPTION_END);

    if (!daemon_)
    {
        logger->error(LTRACE, "%s server failed to start on port %d", server_name_.c_str(), port_);
        return false;
    }
    running_cv_.notify_all(); //daemon_ has changed value

    logger->info(LTRACE, "%s server created on port %d", server_name_.c_str(), port_);

    return true;
}


bool server::server_impl::is_running()
{
    return (daemon_ != nullptr);
}

void server::server_impl::stop()
{
    if (daemon_)
    {
        MHD_stop_daemon(daemon_);
        logger->info(LTRACE, "%s server stopped", server_name_.c_str());
        daemon_ = nullptr;
        running_cv_.notify_all(); //daemon_ has changed value
    }
}

void server::server_impl::await()
{
    std::mutex m;
    {
        std::unique_lock<std::mutex> lk(m);
        running_cv_.wait(lk, [this]
        { return daemon_ == nullptr; });
    }
}


uint16_t server::server_impl::get_port()
{
    return port_;
}

std::shared_ptr<router> server::server_impl::create_router(std::string route_base)
{
    std::shared_ptr<router> r{new router{route_base}};
    routers_.emplace_back(r);
    return r;
}


//////// option setters

void server::server_impl::set_option_(debug_output value)
{
    debug_output_ = static_cast<bool>(value);
}

void server::server_impl::set_option_(use_thread_per_connection value)
{
    use_thread_per_connection_ = static_cast<bool>(value);
    if (use_epoll_if_available_)
    {
        logger->info(LTRACE, "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_epoll_if_available");
        use_epoll_if_available_ = false; //not compatible!
    }
}

void server::server_impl::set_option_(use_epoll_if_available value)
{
    use_epoll_if_available_ = static_cast<bool>(value);
    if (use_thread_per_connection_)
    {
        logger->info(LTRACE, "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_thread_per_connection");
        use_thread_per_connection_ = false; //not compatible!
    }
}

void server::server_impl::set_option_(accept_policy_cb value)
{
    accept_policy_callback_ = value;
}

void server::server_impl::set_option_(connection_memory_limit value)
{
    //this is a narrowing cast, so ugly! What to do, though?
    options_.push_back({MHD_OPTION_CONNECTION_MEMORY_LIMIT, static_cast<intptr_t>(value), NULL});
}

void server::server_impl::set_option_(connection_limit value)
{
    options_.push_back({MHD_OPTION_CONNECTION_LIMIT, static_cast<intptr_t>(value), NULL});
}

void server::server_impl::set_option_(connection_timeout value)
{
    options_.push_back({MHD_OPTION_CONNECTION_TIMEOUT, static_cast<intptr_t>(value), NULL});
}

//void server::server_impl::set_option_(notify_completed value)
//{
//    //TODO
//}

void server::server_impl::set_option_(per_ip_connection_limit value)
{
    options_.push_back({MHD_OPTION_PER_IP_CONNECTION_LIMIT, static_cast<intptr_t>(value), NULL});
}

void server::server_impl::set_option_(const sockaddr_ptr value)
{
    //why are we casting away the constness? Because MHD isn'T going to modify this, and I want the caller
    // to be assured of this fact.
    options_.push_back({MHD_OPTION_SOCK_ADDR, 0, const_cast<sockaddr *>(value)});
}

//void server::server_impl::set_option_(uri_log_callback value)
//{
//    options_.push_back({MHD_OPTION_URI_LOG_CALLBACK, value, NULL});
//}

void server::server_impl::set_option_(const server::https_mem_key &value)
{
    // we must make a durable copy of these strings before tossing around char pointers to their internals
    https_mem_key_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_MEM_KEY, 0,
                               const_cast<char *>(https_mem_key_.back().c_str())});
    ssl_mem_key_set_ = true;
}

void server::server_impl::set_option_(const server::https_mem_cert &value)
{
    https_mem_cert_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_MEM_CERT, 0,
                               const_cast<char *>(https_mem_cert_.back().c_str())});
    ssl_mem_cert_set_ = true;
}

//void server::server_impl::set_option_(https_cred_type value)
//{
//    //TODO
//}

void server::server_impl::set_option_(const server::https_priorities &value)
{
    https_priorities_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_PRIORITIES, 0,
                               const_cast<char *>(https_priorities_.back().c_str())});
}

void server::server_impl::set_option_(listen_socket value)
{
    options_.push_back({MHD_OPTION_LISTEN_SOCKET, static_cast<intptr_t>(value), NULL});
}

void server::server_impl::set_option_(thread_pool_size value)
{
    options_.push_back({MHD_OPTION_THREAD_POOL_SIZE, static_cast<intptr_t>(value), NULL});
}

void server::server_impl::set_option_(unescaper_cb value)
{
    unescaper_callback_ = value;
    options_.push_back({MHD_OPTION_UNESCAPE_CALLBACK, (intptr_t) &(unescaper_callback_shim_), this});
}

//void server::server_impl::set_option_(digest_auth_random value)
//{
//    //TODO
//}

void server::server_impl::set_option_(nonce_nc_size value)
{
    options_.push_back({MHD_OPTION_NONCE_NC_SIZE, static_cast<intptr_t>(value), NULL});
}

void server::server_impl::set_option_(thread_stack_size value)
{
    options_.push_back({MHD_OPTION_THREAD_STACK_SIZE, static_cast<intptr_t>(value), NULL});
}

void server::server_impl::set_option_(const server::https_mem_trust &value)
{
    https_mem_trust_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_MEM_TRUST, 0,
                               const_cast<char *>(https_mem_trust_.back().c_str())});
}

void server::server_impl::set_option_(connection_memory_increment value)
{
    options_.push_back({MHD_OPTION_CONNECTION_MEMORY_INCREMENT, static_cast<intptr_t>(value), NULL});
}

//void server::server_impl::set_option_(https_cert_callback value)
//{
//    //TODO
//}

//void server::server_impl::set_option_(tcp_fastopen_queue_size value)
//{
//    options_.push_back({MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE, value, NULL});
//}

void server::server_impl::set_option_(const server::https_mem_dhparams &value)
{
    https_mem_dhparams_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_MEM_DHPARAMS, 0,
                               const_cast<char *>(https_mem_dhparams_.back().c_str())});
}

//void server::server_impl::set_option_(listening_address_reuse value)
//{
//    options_.push_back({MHD_OPTION_LISTENING_ADDRESS_REUSE, value, NULL});
//}

void server::server_impl::set_option_(const server::https_key_password &value)
{
    https_key_password_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_KEY_PASSWORD, 0,
                               const_cast<char *>(https_key_password_.back().c_str())});
}

//void server::server_impl::set_option_(notify_connection value)
//{
//    //TODO
//}

void server::server_impl::set_option_(const server::server_identifier &value)
{
    response_renderer_.set_option(value);
    std::string id = value; //because it is not really a string
    server_name_ = id.substr(0, id.find("/"));
}

void server::server_impl::set_option_(const server::server_identifier_and_version &value)
{
    response_renderer_.set_option(value);
    server_name_ = value.first;
}

void server::server_impl::set_option_(const server::append_to_server_identifier &value)
{
    response_renderer_.set_option(value);
}

void server::server_impl::set_option_(enable_internal_file_cache value)
{
    response_renderer_.set_option(value);
}

void server::server_impl::set_option_(internal_file_cache_keep_alive value)
{
    response_renderer_.set_option(value);
}

void server::server_impl::set_option_(not_found_handler_cb value)
{
    // At the moment, there are multiple places where we might generate a 404:
    //  In the server, when no router responds to a request
    //  In the response renderer, when a file isn't found.
    // So for now, set this handler in both places.
    // Not very fond of this implementation! It requires us to keep two different places in sync.
    // Also, it requires two different default functions.
    // TODO make this better.
    // TODO also, we should do something similar for 500 errors, to generate traces and such
    not_found_handler_ = value;
    response_renderer_.set_option(value);
}



//////// private methods setters

struct connection_info_struct
{
    request_method connectiontype;
    query_params post_params;
    std::string body;
    MHD_PostProcessor *postprocessor;

    connection_info_struct(request_method method,
                           struct MHD_Connection *connection,
                           size_t buffer_size,
                           MHD_PostDataIterator iter) :
            connectiontype{method}, postprocessor{nullptr}
    {
        postprocessor = MHD_create_post_processor(connection, buffer_size, iter, this);
    }

    ~connection_info_struct()
    {
        if (postprocessor)
        {
            MHD_destroy_post_processor(postprocessor);
        }
    }
};

request_method method_str_to_enum_(const char *method_str)
{
    if (!std::strcmp(method_str, "GET"))
    {
        return request_method::GET;
    }

    if (!std::strcmp(method_str, "PUT"))
    {
        return request_method::PUT;
    }

    if (!std::strcmp(method_str, "POST"))
    {
        return request_method::POST;
    }

    if (!std::strcmp(method_str, "PATCH"))
    {
        return request_method::PATCH;
    }

    if (!std::strcmp(method_str, "DELETE"))
    {
        return request_method::DELETE;
    }

    if (!std::strcmp(method_str, "OPTIONS"))
    {
        return request_method::OPTIONS;
    }

    return request_method::UNKNOWN;
}

//TODO I hate this.
request_method method_str_to_enum_(const std::string &method_str)
{
    return method_str_to_enum_(method_str.c_str());
}

std::string addr_to_str_(const struct sockaddr *addr)
{
    if (addr)
    {
        char str[INET_ADDRSTRLEN];

        switch (addr->sa_family)
        {
            case AF_INET:
                inet_ntop(addr->sa_family,
                          &(reinterpret_cast<const sockaddr_in *>(addr)->sin_addr),
                          str,
                          INET_ADDRSTRLEN);
                break;
            case AF_INET6:
                inet_ntop(addr->sa_family,
                          &(reinterpret_cast<const sockaddr_in6 *>(addr)->sin6_addr),
                          str,
                          INET_ADDRSTRLEN);
                break;
            default:
                return "";
        }
        return std::string{str};
    }
    return "";
}

MHD_ValueKind method_to_value_kind_enum_(request_method method)
{
    if (method == request_method::GET)
    {
        return MHD_GET_ARGUMENT_KIND;
    }

    return MHD_POSTDATA_KIND;
}


int parse_kv_(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
    switch (kind)
    {
        case MHD_HEADER_KIND:
        // case MHD_RESPONSE_HEADER_KIND:
        {
            auto kv = static_cast<case_insensitive_map *>(cls);
            (*kv)[key] = value ? value : "";
        }
            break;
        default:
        {
            auto kv = static_cast<case_sensitive_map *>(cls);
            (*kv)[key] = value ? value : "";
        }
    }
    return MHD_YES;
}

void parse_post_text_params_(connection_info_struct *con_info, std::string& url_str, query_params& query_params)
{
    const std::string& body = con_info->body;
    int len = body.length();
    int start = 0;
    for (; start < len; ) {
        int end = body.find("&", start);
        if (end == std::string::npos) end = len;

        int mid = body.find("=", start);

        if (mid == std::string::npos || mid >= end) break;

        std::string key = body.substr(start, mid - start);
        std::string val = body.substr(mid + 1, end - mid - 1);
        val.erase(0, val.find_first_not_of("\""));
        val.erase(val.find_last_not_of("\"") + 1);

        query_params.insert(std::pair<std::string, std::string>(key, val));

        start = end + 1;
    }
}

void parse_post_json_params_(connection_info_struct *con_info, std::string& url_str, query_params& query_params)
{
    const std::string& body = con_info->body;
    con_info->post_params;
    if (!nlohmann::json::accept(body))
    {
        logger->error(LTRACE, "url:%s, post json data illegal ......", url_str.c_str());
        logger->error(LTRACE, "post data: %s", body.c_str());
        return;
    }

    logger->info(LTRACE, "url: %s", url_str.c_str());
    logger->info(LTRACE, "post data: %s", body.c_str());

    nlohmann::json body_json = nlohmann::json::parse(body);
    nlohmann::json::iterator it = body_json.begin();
    nlohmann::json::iterator end = body_json.end();
    for ( ; it != end; ++it)
    {
        query_params.insert(std::pair<std::string, nlohmann::json>(it.key(), it.value()));
    }
}

void parse_post_params_(connection_info_struct *con_info, zz_tools::headers& header,
                        std::string& url_str, query_params& query_params)
{
    if (!con_info) return;
    if (con_info->body.empty()) return;

    std::string& content_type = header.at("Content-Type");
    if (content_type.find(CONTENT_JSON) != std::string::npos) {
        parse_post_json_params_(con_info, url_str, query_params);
    } else {
        parse_post_text_params_(con_info, url_str, query_params);
    }
}

int server::server_impl::access_handler_callback_(struct MHD_Connection *connection,
                                                  const char *url,
                                                  const char *method_char,
                                                  const char *version,
                                                  const char *upload_data,
                                                  size_t *upload_data_size,
                                                  void **con_cls)
{
    auto start = std::chrono::system_clock::now();

    std::string http_version{version};

    request_method method = method_str_to_enum_(method_char);
    std::string method_str{method_char};

    std::string url_str{url};

    if (!*con_cls)
    {
        connection_info_struct *con_info = new(std::nothrow) connection_info_struct(method,
                                                                                    connection,
                                                                                    65535,
                                                                                    iterate_postdata_shim_);
        if (!con_info) return MHD_NO; //TODO what does this mean?

        *con_cls = con_info;

        return MHD_YES;
    }

    //parse the query params:
    zz_tools::headers header;

    MHD_get_connection_values(connection, MHD_HEADER_KIND, &parse_kv_, &header);

    //find the route, and hit the right callback
    query_params query_params;

    //Query params handling
    MHD_get_connection_values(connection, method_to_value_kind_enum_(method), &parse_kv_, &query_params);

    //POST data handling. This is a tortured flow, and not really MHD' high point.
    auto con_info = static_cast<connection_info_struct *>(*con_cls);
    if (method == request_method::POST)
    {
        // auto con_info = static_cast<connection_info_struct *>(*con_cls);
        if (*upload_data_size != 0)
        {
            //TODO note that we just drop BINARY data on the floor at present!! See iterate_postdata_shim_()
            if (MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size) == MHD_NO)
            {
                //MHD couldn't parse it, maybe we can.
                con_info->body.append(upload_data, *upload_data_size);
            }

            *upload_data_size = 0; //flags that we processed everything. This is a funny place to put it.
            return MHD_YES;
        }

        parse_post_params_(con_info, header, url_str, query_params);
        // if (!con_info->post_params.empty())//we're done getting postdata, and we have some query params to handle, do something with it
        // {
        //     //if we have post_params, then MHD has ignored the query params. So just overwrite it.
        //     std::swap(query_params, con_info->post_params);
        //     logger->info(LTRACE, "swap post params ......");
        // }
    }

    // construct request object
    auto ip_address = addr_to_str_(MHD_get_connection_info(connection,
                                                           MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr);

    zz_tools::request request{start, start, ip_address, method, url_str, http_version, {}, query_params, header,
                          con_info->body};

    logger->debug(LTRACE, "Received request for %s%s", method_str.c_str(), url_str.c_str());


    //iterate through the handlers. Could stand being parallelized, I suppose?
    OPT_NS::optional<response> response;

    // std::unique_lock<std::mutex> ulock{lock_};
    for (auto &router : routers_)
    {
        response = router->process_request(request);
        if(response)
        {
            break;
        }
    }
    // ulock.unlock();

    if (!response)
    {
        // if there was no response generated by a request handler, make us a 404.
        response = zz_tools::response{404, "text/html; charset=utf-8", "<html><h1>404 Not Found</h1></html>"};

        if (not_found_handler_)
        {
            not_found_handler_(request, *response);
        }
    }

    // TODO this is the point where we will want to include middlewares in the future.

    auto response_mhd = response_renderer_.render(request, *response);
    auto retval = MHD_queue_response(connection, response_mhd->status_code, response_mhd->mhd_response);

    request.end = std::chrono::system_clock::now();

    // log it
    auto end_c = std::chrono::system_clock::to_time_t(request.end);
    auto tm = zz_tools::gmtime(end_c);

    logger->debug(LTRACE, "%s: %s [%d] %s %s %s {%d us}",
                  request.ip_address.c_str(),
                  zz_tools::to_string(request.method).c_str(),
                  response->status_code,
                  request.path.c_str(), request.http_version.c_str(),
                  request.headers.count("user-agent") ? request.headers.at("user-agent").c_str() : "[no user-agent]",
                  std::chrono::duration_cast<std::chrono::microseconds>(request.end - request.start).count());

    return retval;
}

/////////// callback shims

int server::server_impl::access_handler_callback_shim_(void *cls,
                                                       struct MHD_Connection *connection,
                                                       const char *url,
                                                       const char *method,
                                                       const char *version,
                                                       const char *upload_data,
                                                       size_t *upload_data_size,
                                                       void **con_cls)
{
    if (!cls) return MHD_NO;

    return static_cast<server_impl *>(cls)->access_handler_callback_(connection,
                                                                     url,
                                                                     method,
                                                                     version,
                                                                     upload_data,
                                                                     upload_data_size,
                                                                     con_cls);
}


int server::server_impl::access_policy_callback_shim_(void *cls, const struct sockaddr *addr, socklen_t addrlen)
{
    if (!cls) return MHD_NO;

    return static_cast<server_impl *>(cls)->accept_policy_callback_(addr, addrlen);
}


void server::server_impl::request_completed_callback_shim_(void *cls, struct MHD_Connection *connection,
                                                           void **con_cls,
                                                           enum MHD_RequestTerminationCode toe)
{
    auto con_info = static_cast<connection_info_struct *>(*con_cls);

    if (con_info && con_info)
    {
        delete con_info;
        *con_cls = NULL;
    }
}

void *server::server_impl::uri_logger_callback_shim_(void *cls, const char *uri, struct MHD_Connection *con)
{
//    LOG_DEBUG(uri); //TODO and stuff about the connection too!
    return nullptr;
}

int server::server_impl::iterate_postdata_shim_(void *cls,
                                                enum MHD_ValueKind kind,
                                                const char *key,
                                                const char *filename,
                                                const char *content_type,
                                                const char *transfer_encoding,
                                                const char *data,
                                                uint64_t off,
                                                size_t size)
{
    auto con_info = static_cast<connection_info_struct *>(cls);
    //TODO this is where we would process binary data. This needs to be implemented
    //TODO unsure how to differentiate between binary (multi-part) post data, and query params, so I am going to wing it
    //  ANnoyingly, when query params are sent here, content_type is nil. As is transfer_encoding. So.

    if (key) //TODO this is a hack, I don't even know if this is a reliable way to detect query params
    {
        auto con_info = static_cast<connection_info_struct *>(cls);
        parse_kv_(&con_info->post_params, kind, key, data);
        return MHD_YES;
    }

    return MHD_YES;
}

void server::server_impl::logger_callback_shim_(void *cls, const char *fm, va_list ap)
{
    //not at all happy with this.
    char message[4096] = {0};
    std::vsnprintf(message, sizeof(message), fm, ap);
    // error_log(log_level::DEBUG, message);
    logger->debug(LTRACE, "%s", message);
}

size_t server::server_impl::unescaper_callback_shim_(void *cls, struct MHD_Connection *c, char *s)
{
    auto this_ptr = static_cast<server_impl *>(cls);
    if (this_ptr && this_ptr->unescaper_callback_)
    {
        auto result = this_ptr->unescaper_callback_(s);
        auto old_len = strlen(s);
        memcpy(s, result.c_str(), old_len);
        return (old_len > result.length()) ? result.length() : old_len;
    }

    return strlen(s); //no change
}

//void server::server_impl::notify_connection_callback_shim_(void *cls,
//                                                           struct MHD_Connection *connection,
//                                                           void **socket_context,
//                                                           enum MHD_ConnectionNotificationCode toe)
//{
//    auto this_ptr = static_cast<server_impl *>(cls);
//    if (this_ptr && this_ptr->notify_connection_callback_)
//    {
//        return this_ptr->notify_connection_callback_(connection, socket_context, toe);
//    }
//}

} //namespace zz_tools
