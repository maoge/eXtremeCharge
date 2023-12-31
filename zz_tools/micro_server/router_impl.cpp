#include "router_impl.h"
#include "config.h"
#include <mutex>
#include <vector>
#include <stack>
#include <algorithm>
#include <lwpr.h>

namespace zz_tools
{

router::router_impl::router_impl(std::string route_base) :
        route_base_{std::move(route_base)},
        mime_type_{"text/html; charset=utf-8"}
{
    //remove trailing slashes
    if (route_base_.back() == '/')
    {
        route_base_.pop_back();
    }
}

void router::router_impl::set_mime_type(std::string mime_type)
{
    mime_type_ = mime_type;
}

void router::router_impl::handle_request(request_method method,
                            std::regex route,
                            router::endpoint_handler_cb callback,
                            parameter::validators validations)
{
    // std::lock_guard<std::mutex> guard{lock_};
    request_handlers_[method].insert(std::end(request_handlers_[method]),
                                            std::make_tuple(
                                                    route,
                                                    callback,
                                                    validations));
}

void router::router_impl::handle_request(request_method method,
                            std::string route,
                            router::endpoint_handler_cb callback,
                            parameter::validators validations)
{
    // std::lock_guard<std::mutex> guard{lock_};
    request_handlers_[method].insert(std::end(request_handlers_[method]),
                                            std::make_tuple(
                                                    std::regex{route},
                                                    callback,
                                                    validations));
}

std::string sanitize_path_(std::string path_to_files)
{
    std::vector<std::string> url_comps;
    std::string delimeter = "/";
    std::string final_path = "";
    std::string working_path = path_to_files;
    size_t pos = 0;
    int num_front_slash = 0, num_back_slash = 0;

    // path can contain many / as prefix or suffix
    // we want to preserve that in the final path
    while (pos < path_to_files.size() and path_to_files[pos++] == '/') {
        ++num_front_slash;
    }

    pos = path_to_files.size() - 1;
    while (int(pos) - 1 >= 0 and path_to_files[pos--] == '/') {
        ++num_back_slash;
    }

    // Now, split by '/' to get URL components
    while ((pos = working_path.find(delimeter)) != std::string::npos) {
        std::string comp =  working_path.substr(0, pos);
        if (not comp.empty()) {
            url_comps.push_back(comp);
        }
        working_path.erase(0, pos + delimeter.length());
    }

    std::string last_comp = working_path.substr(0, pos);
    if (not last_comp.empty()) {
        url_comps.push_back(last_comp);
    }

    if (url_comps.empty()) {
        // nothing to sanitize
        return path_to_files;
    }

    std::stack<std::string> stk;
    for (auto& comp : url_comps) {
        if (comp == ".." and stk.empty()) {
            // trying to access a file outside cwd
            // Return empty string which would result in 404
            return "";
        } else if (comp == "..") {
            stk.pop();
        } else {
            stk.push(comp);
        }
    }

    // Build final path from stack
    while (not stk.empty()) {
        final_path = stk.top() + delimeter + final_path;
        stk.pop();
    }

    // remove trailing '/' added by previous stmt
    final_path.pop_back();

    // add back the /s in
    while (num_front_slash--) {
        final_path = "/" + final_path;
    }

    while (num_back_slash--) {
        final_path += "/";
    }

    return final_path;
}

void router::router_impl::serve_files(std::string mount_point, std::string path_to_files)
{
    path_to_files = sanitize_path_(path_to_files);
    std::regex route{mount_point + "(.*)"};
    std::string local_path{path_to_files + "/"};
    handle_request(request_method::GET, route, [=](const request &req) -> response
    {
        std::string path = local_path + req.matches[1];

        logger->debug(LTRACE, "File requested:  %s", req.matches[1].c_str());
        logger->debug(LTRACE, "Serve from    :  %s", path.c_str());

        return response::from_file(path);
    });
}

void router::router_impl::add_header(std::string &&key, std::string &&value)
{
    headers_[key] = std::move(value);
}


// Helper function to tack on headers
zz_tools::response make_response_(zz_tools::response &&response, zz_tools::headers &headers_)
{
    for(const auto header : headers_)
    {
        // don't override anything already here
        if(response.headers.count(header.first) == 0)
        {
            response.headers[header.first] = header.second;
        }
    }
    return response;
}

OPT_NS::optional<zz_tools::response> router::router_impl::process_request(request &request)
{
    // TODO this is here to prevent writing to the list of endpoints while we're using it. Not sure we actually need this,
    // if we can find a way to restrict writing to the list of endpoints when the server is running.
    // we need this because our iterators can get invalidated by a concurrent insert. The insert must wait until after we are done.
    // std::unique_lock<std::mutex> ulock{lock_};

    // first lets validate that the path begins with our base_route_, and if it does, strip it from the request to simplify the logic below
    if (!std::regex_search(request.path, std::regex{"^" + route_base_}))
    {
        return OPT_NS::nullopt;
    }

    //strip the base_path_ off the reqest
    auto path = request.path.substr(route_base_.length(), std::string::npos);

    OPT_NS::optional<zz_tools::response> response;

    for (const auto &handler_tuple : request_handlers_[request.method])
    {
        std::smatch pieces_match;
        auto path_regex = std::get<std::regex>(handler_tuple);

        if (std::regex_match(path, pieces_match, path_regex))
        {
            // ulock.unlock(); // found a match, can unlock as we won't continue down the list of endpoints.

            std::vector<std::string> matches;
            logger->debug(LTRACE, "    match: %s", path.c_str());
            for (size_t i = 0; i < pieces_match.size(); ++i)
            {
                std::ssub_match sub_match = pieces_match[i];
                std::string piece = sub_match.str();
                logger->debug(LTRACE, "      submatch %d%s", i, piece.c_str());
                
                matches.emplace_back(sub_match.str());
            }

            request.matches = matches;

            auto callback = std::get<endpoint_handler_cb>(handler_tuple);
            try
            {
                // Validate the parameters passed in
                // TODO this can probably be optimized
                // TODO refactor this out!
                bool valid_params{true};
                auto validators = std::get<parameter::validators>(handler_tuple);
                for (const auto &validator : validators)
                {
                    bool present = (request.params.count(validator.key) == 0) ? false : true;
                    if (present)
                    {
                        if (validator.validation_func == nullptr) continue;
                        
                        //run the validator
                        if (!validator.validation_func(request.params[validator.key]))
                        {
                            std::string error{"Request handler for \"" + path + "\" is missing required parameter \"" + validator.key + "\""};
                            logger->error(LTRACE, error.c_str());
                            response = make_response_({400, "text/plain", error}, headers_);
                            valid_params = false;
                            break; //stop examining params
                        }
                    }
                    else if (validator.required) //not present, but required
                    {
                        std::string error{"Request handler for \"" + path + "\" is missing required parameter \"" + validator.key + "\""};
                        logger->error(LTRACE, error.c_str());
                        response = make_response_({400, "text/plain", error}, headers_);
                        valid_params = false;
                        break; //stop examining params
                    }
                }

                if (valid_params)
                {
                    //made it this far! try the callback
                    response = make_response_(callback(request), headers_);

                    // add mime type if needed. Don't add a mimetype for file responses
                    if (response->file.empty() && response->content_type.empty()) //no content type assigned, use the default
                    {
                        response->content_type = mime_type_;
                    }
                }
            }
            catch (const std::exception &e)
            {
                logger->error(LTRACE, "Request handler for \" %s \" threw an exception: %s", path.c_str(), e.what());
                response = make_response_({500, "text/plain", "Internal error"}, headers_);
                //TODO render the stack trace, etc.
            }
            catch (...)
            {
                logger->error(LTRACE, "Unknown internal error");
                //TODO use the same error message as above, and just log things differently and test for that.
                response = make_response_({500, "text/plain", "Unknown internal error"}, headers_);
                //TODO render the stack trace, etc.
            }

            break; //exit the for loop iterating over all the request handlers
        }
    }

    return response;
}

}
