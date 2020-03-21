#pragma once
#include <unordered_map>

namespace Request {
    enum class Name {
        STOP,
        BUS,
        ROUTE,
        WAIT
    };


}
enum class RouteType {
    CIRCULAR,
    LOOPING
};

static const std::unordered_map<std::string, Request::Name> requestNames = {
        {"Stop",  Request::Name::STOP},
        {"Bus",   Request::Name::BUS},
        {"Route", Request::Name::ROUTE},
        {"Wait", Request::Name::WAIT}
};
static const std::unordered_map<Request::Name, std::string> requestIds = {
        {Request::Name::STOP, "Stop" },
        {Request::Name::BUS,   "Bus"},
        {Request::Name::ROUTE, "Route"},
        {Request::Name::WAIT, "Wait"}
};
