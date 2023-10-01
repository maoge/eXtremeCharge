#pragma once

#include <string>
#include <unordered_map>

namespace zz_tools
{

static const std::unordered_map <std::string, std::string> mime_types
        {
                {"html", "text/html"},
                {"htm", "text/html"},
                {"css", "text/css"},
                {"js",  "text/javascript"},
        };

static const std::vector <std::string> index_filenames
        {
                "index.html",
                "index.htm",
        };
} //namespace zz_tools
