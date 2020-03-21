#ifndef BROWNFINAL_DATABASE_H
#define BROWNFINAL_DATABASE_H
#include <unordered_map>
#include <istream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <unordered_set>
#include <optional>
#include <set>
#include <future>
#include "graph.h"
#include "router.h"
#include "json.hpp"
#include <cstdlib>
#include <utility>
#include <deque>



class Coordinates {
    double latitude, longitude;
    constexpr static double factor = 3.1415926535 / 180;

public:
    Coordinates() = default;

    Coordinates(double latitude, double longitude) : latitude(latitude * factor), longitude(longitude * factor) {}

    [[nodiscard]] double CalcDist(const Coordinates &other) const noexcept {
        return acos(
                sin(latitude) * sin(other.latitude) +
                cos(latitude) * cos(other.latitude) * cos(std::abs(longitude - other.longitude))
        ) * 6371000;
    }

    bool operator==(const Coordinates &other) const noexcept {
        return latitude == other.latitude && longitude == other.longitude;
    }

};




struct RouteInfo {
    RouteType type;
    std::vector<std::string> stops;

    bool operator==(const RouteInfo &other) const {
        return type == other.type && stops == other.stops;
    }

    RouteInfo() = default;

    RouteInfo(RouteType type, std::vector<std::string> stops) : type(type), stops(std::move(stops)) {}

    RouteInfo(RouteInfo &&other) noexcept : type(other.type), stops(std::move(other.stops)) {}
};



class DataBase {
    struct StopData;
    using StopStats = std::unordered_map<std::string, StopData>;
    struct RouteData;
    using RouteStats = std::unordered_map<std::string, RouteData>;


    struct RouteData {
        using distOptional = std::optional<std::vector<unsigned>>;

        explicit RouteData(RouteInfo routeInfo) : type(routeInfo.type), num_stops(routeInfo.stops.size()),
                                                  stops(std::move(routeInfo.stops)),
                                                  unique_stops(stops.begin(), stops.end()) {}

        void InitDists(const StopStats &stop_coords) {
            dists.emplace(distOptional::value_type());
            switch (type) {
                case RouteType::LOOPING:
                    num_stops+= num_stops - 1;
                    dists->reserve(num_stops - 1);
                    CalcLoopingDist(stop_coords);
                    break;
                default:
                    dists->reserve(num_stops - 1);
                    CalcCircularDist(stop_coords);
            }
        }

    private:
        void CalcLoopingDist(const StopStats &stop_coords) {
            const auto stops_begin = stops.begin(), stops_end = stops.end();
            auto prev_stop_data = &stop_coords.at(*stops_begin);
            auto prev_stop = stops_begin;
            std::vector<unsigned> rev_dists;
            rev_dists.reserve(stops.size() - 1);
            for (auto stop = std::next(stops_begin); stop != stops_end; ++stop) {
                auto cur_stop_data = &stop_coords.at(*stop);
                auto geo_dist_addend = cur_stop_data->coordinates.CalcDist(prev_stop_data->coordinates);
                geo_distance += geo_dist_addend;

                auto it = prev_stop_data->distances.find(*stop);
                if (it != prev_stop_data->distances.end()) {
                    real_distance += it->second;
                    dists->push_back(it->second);
                    if (auto it2 = cur_stop_data->distances.find(*prev_stop); it2 !=
                                                                              cur_stop_data->distances.end()) {
                        real_distance += it2->second;
                        rev_dists.push_back(it2->second);
                    } else {
                        real_distance += it->second;
                        rev_dists.push_back(it->second);
                    }
                } else {
                    auto &real_dist = cur_stop_data->distances.at(*prev_stop);
                    real_distance += real_dist;
                    real_distance += real_dist;
                    dists->push_back(real_dist);
                    rev_dists.push_back(real_dist);
                }

                prev_stop_data = cur_stop_data;
                prev_stop = stop;
            }
            geo_distance += geo_distance;
            std::copy(rev_dists.rbegin(), rev_dists.rend(), std::back_inserter(*dists));
        }

        void CalcCircularDist(const StopStats &stop_coords) {
            const auto stops_begin = stops.begin(), stops_end = stops.end();
            auto prev_stop_data = &stop_coords.at(*stops_begin);
            auto prev_stop = stops_begin;
            for (auto stop = std::next(stops_begin); stop != stops_end; ++stop) {
                auto cur_stop_data = &stop_coords.at(*stop);
                auto geo_dist_addend = cur_stop_data->coordinates.CalcDist(prev_stop_data->coordinates);
                geo_distance += geo_dist_addend;

                if (auto it = prev_stop_data->distances.find(*stop); it != prev_stop_data->distances.end()) {
                    real_distance += it->second;
                    dists->push_back(it->second);
                } else {
                    auto &real_dist_addend = cur_stop_data->distances.at(*prev_stop);
                    real_distance += real_dist_addend;
                    dists->push_back(real_dist_addend);
                }
                prev_stop_data = cur_stop_data;
                prev_stop = stop;
            }
        }

    public:
        RouteType type;
        unsigned num_stops, real_distance = 0;
        double geo_distance = 0.0;

        std::vector<std::string> stops;
        std::unordered_set<std::string_view> unique_stops;
        mutable distOptional dists = std::nullopt;
    };

    struct StopData {
        Coordinates coordinates;
        std::optional<std::set<std::string_view>> route_set_proxy;
        std::unordered_map<std::string, unsigned> distances;

        void InitProxy(const std::string &stop_name, const RouteStats &routeStats) {
            route_set_proxy = std::set<std::string_view>();
            for (auto &[route, routeData] : routeStats) {
                for (auto &stop : routeData.stops) {
                    if (stop == stop_name) {
                        route_set_proxy->emplace(route);
                        break;
                    }
                }
            }
        }
    };

    struct TemporalInfo {
        double waitTime, velocity;
    };

    class GraphBuilder {
        using WeightType = double;
        using Router = Graph::Router<WeightType>;
    public:
        GraphBuilder(const RouteStats &routeStats, const StopStats &stopStats, const TemporalInfo &temporalInfo) :
                graph(stopStats.size()),
                nameToOutStopID(InitGraphAndMaxEdgeIDtoRouteName(routeStats, temporalInfo)),
                router(std::make_shared<Graph::DirectedWeightedGraph<double>>(graph)) {
            FillnameToOutStopID(nameToOutStopID.size(), stopStats);
        }

        GraphBuilder(const GraphBuilder &g) : graph(g.graph),
                                              nameToOutStopID(g.nameToOutStopID),
                                              router(g.router) {}
        GraphBuilder(GraphBuilder &&g) :      router(std::move(g.router)),
                                              nameToOutStopID(std::move(g.nameToOutStopID)),
                                              graph(g.graph)
        {
            router.graph_ = std::make_shared<Graph::DirectedWeightedGraph<double>>(graph);
        }
        template<typename It>
        struct ItRange {
            It begin, end;
        };

        Graph::DirectedWeightedGraph<double> graph;
        std::unordered_map<std::string_view, unsigned> nameToOutStopID;
        Router router;
    private:
        void FillnameToOutStopID(size_t inStopID, const StopStats &stopStats){
            for (const auto &el: stopStats){
                if (nameToOutStopID.find(el.first) == nameToOutStopID.end()) {
                    nameToOutStopID.insert({el.first, inStopID++});
                }
            }
        }
        std::unordered_map<std::string_view, unsigned>
        InitGraphAndMaxEdgeIDtoRouteName(const RouteStats &routeStats, const TemporalInfo temporalInfo) {
            std::unordered_map<std::string_view, unsigned> result;
            unsigned inStopID = 0;
            for (auto &[routeName, routeData]: routeStats) {
                auto stops_it = routeData.stops.begin();
                if (result.find(*stops_it) == result.end()) {
                    result.insert({*stops_it, inStopID++});
                }
                switch (routeData.type) {
                    case RouteType::CIRCULAR:
                        __InitInnerPartOfCircularRoute(
                                {stops_it, routeData.stops.end()},
                                *routeData.dists,
                                inStopID,
                                temporalInfo, routeName, result
                        );
                        break;
                    default:
                        __InitInnerPartOfLoopingRoute(
                                {stops_it, routeData.stops.end()},
                                *routeData.dists,
                                inStopID,
                                temporalInfo,
                                routeData.unique_stops.size() - 1, routeName, result
                        );
                }
            }
            return result;
        }


        void __InitInnerPartOfLoopingRoute(ItRange<std::vector<std::string>::const_iterator> inner_stops,
                                           const std::vector<unsigned> &dists,
                                           unsigned &inStopID,
                                           const TemporalInfo temporalInfo,
                                           unsigned stops_num, std::string_view routeName,
                                           std::unordered_map<std::string_view, unsigned>& result) {

            auto dists_it = dists.begin();
            for (auto this_stop = inner_stops.begin;this_stop != std::prev(inner_stops.end); this_stop++) {
                auto this_stop_id = result[*this_stop];
                auto dists_from_this_vartex = dists_it;
                double total_dist = temporalInfo.waitTime;
                size_t span_count = 1;
                for (auto vartex_to = std::next(this_stop); vartex_to != inner_stops.end; vartex_to++) {
                    if (result.find(*vartex_to) == result.end()){
                        result.insert({*vartex_to, inStopID++});
                    }
                    total_dist += *(dists_from_this_vartex++) / temporalInfo.velocity;
                    graph.AddEdge(
                            {this_stop_id, result[*vartex_to],
                             total_dist,
                             span_count++, routeName, *this_stop}
                    );
                }
                dists_it++;
            }
            for (auto this_stop = std::prev(inner_stops.end);this_stop != inner_stops.begin; this_stop--) {
                auto this_stop_id = result[*this_stop];
                auto dists_from_this_vartex = dists_it;
                double total_dist = temporalInfo.waitTime;
                size_t span_count = 1;
                for (auto vartex_to = std::prev(this_stop); vartex_to != inner_stops.begin; vartex_to--) {
                    if (result.find(*vartex_to) == result.end()){
                        result.insert({*vartex_to, inStopID++});
                    }
                    total_dist += *(dists_from_this_vartex++) / temporalInfo.velocity;
                    graph.AddEdge(
                            {this_stop_id, result[*vartex_to],
                             total_dist,
                             span_count++, routeName, *this_stop}
                    );

                }
                graph.AddEdge(
                        {this_stop_id, result[*inner_stops.begin],
                         total_dist + *(dists_from_this_vartex++) / temporalInfo.velocity,
                         span_count++, routeName, *this_stop});
                dists_it++;
            }
        }

        void __InitInnerPartOfCircularRoute(ItRange<std::vector<std::string>::const_iterator> inner_stops,
                                            const std::vector<unsigned> &dists,
                                            unsigned &inStopID,
                                            const TemporalInfo temporalInfo,
                                            std::string_view routeName, std::unordered_map<std::string_view, unsigned>& result) {
            auto dists_it = dists.begin();
            for (auto this_stop = inner_stops.begin;this_stop != std::prev(inner_stops.end); this_stop++) {
                auto this_stop_id = result[*this_stop];
                auto dists_from_this_vartex = dists_it;
                double total_dist = temporalInfo.waitTime;
                size_t span_count = 1;
                for (auto vartex_to = std::next(this_stop); vartex_to != inner_stops.end; vartex_to++) {
                    if (result.find(*vartex_to) == result.end()){
                        result.insert({*vartex_to, inStopID++});
                    }
                    total_dist += *(dists_from_this_vartex++) / temporalInfo.velocity;
                    graph.AddEdge(
                            {this_stop_id, result[*vartex_to],
                             total_dist,
                             span_count++, routeName, *this_stop}
                    );
                }
                dists_it++;
            }
        }
    };

    std::istream &input;
    std::ostream &output;
    RouteStats route_stats;
    StopStats stop_stats;

    std::optional<GraphBuilder> graph = std::nullopt;
    TemporalInfo settings;
public:

    DataBase(std::istream &input, std::ostream &output) : input(input), output(output) {
        output.precision(6);
    }

    void FillDB(const nlohmann::json &requests) {
        ProcessSettings(requests["routing_settings"]);
        for (auto &request: requests["base_requests"]) {
            switch (requestNames.at(request["type"])) {
                case Request::Name::BUS:
                    ProcessNewRoute(request);
                    break;
                default:
                    ProcessNewBusStop(request);
            }
        }
        for (auto &route: route_stats) {
            route.second.InitDists(stop_stats);
        }
        graph.emplace(GraphBuilder(route_stats, stop_stats, settings));
    }



    nlohmann::json ProcessStatQueries(const nlohmann::json &requests) {
        auto response = nlohmann::json::array();
        std::vector<std::future<nlohmann::json>> vf;
        for (auto &request: requests) {
            switch (requestNames.at(request["type"])) {
                case Request::Name::BUS:
                    vf.push_back(std::async([this, &request](){
                        return ProcessExistingRoute(request);
                    }));
                    break;
                case Request::Name::STOP:
                    vf.push_back(std::async([this, &request](){
                        return ProcessExistingStop(request);
                    }));
                    break;
                default:
                    vf.push_back(std::async([this, &request](){
                        return BuildRoute(request);
                    }));
            }
        }
        for (auto& el: vf){
            response.emplace_back(std::move(el.get()));
        }
        return response;
    }


    DataBase &ProcessJSON() {
        nlohmann::json json_input;
        input >> json_input;
        FillDB(json_input);
        output << ProcessStatQueries(json_input["stat_requests"]) << '\n';
        return *this;
    }

public:
    nlohmann::json BuildItem (Items& it){
        if (it.type == requestNames.at("Bus")) {
            return {
                    {"time",       it.time},
                    {"bus",        it.bus.value()},
                    {"span_count", it.span_count.value()},
                    {"type",       requestIds.at(it.type)}
            };
        } else {
            return {
                    {"time",      it.time},
                    {"stop_name", it.bus.value()},
                    {"type",      requestIds.at(it.type)}
            };
        }
    }
    nlohmann::json BuildRoute(const nlohmann::json &request) {
        auto &nameToOutStopID = graph->nameToOutStopID;
        if ((int)request["id"] == 16){
            int i= 0;
        }
        auto it = graph->router.BuildRoute(
                nameToOutStopID.at((std::string) request["from"]),
                nameToOutStopID.at((std::string) request["to"]),
                settings.waitTime);
        if (it){
            auto items = nlohmann::json::array();
            for(int i = it->it.size() - 1; i>=0; i--){
                items.emplace_back(BuildItem(it->it[i]));
            }
            return {
                    {"request_id",        request["id"]},
                    {"items",        std::move(items)},
                    {"total_time", it->weight}
            };
        }
        else {
            return {
                    {"request_id",    request["id"]},
                    {"error_message", "not found"}
            };
        }
    }


    void ProcessNewRoute(const nlohmann::json &request) {
        route_stats.emplace(
                request["name"],
                RouteData{
                        RouteInfo{
                                (request["is_roundtrip"]) ? RouteType::CIRCULAR : RouteType::LOOPING,
                                request["stops"]
                        }
                }
        );
    }

    void ProcessSettings(const nlohmann::json &request) {
        constexpr auto convertion_multiplier = 1000.0 / 60;
        settings.waitTime = request["bus_wait_time"];
        settings.velocity = (double) request["bus_velocity"] * convertion_multiplier;
    }


    nlohmann::json ProcessExistingRoute(const nlohmann::json &request) {
        const auto it = route_stats.find(request["name"]);
        if (it != route_stats.end()) {
            const auto &stats = it->second;
            return {
                    {"request_id",        request["id"]},
                    {"stop_count",        stats.num_stops},
                    {"unique_stop_count", stats.unique_stops.size()},
                    {"route_length",      stats.real_distance},
                    {"curvature",         stats.real_distance / stats.geo_distance}
            };
        }
        return {
                {"request_id",    request["id"]},
                {"error_message", "not found"}
        };
    }


    nlohmann::json ProcessExistingStop(const nlohmann::json &request) {
        const std::string &name = request["name"];
        const auto it = stop_stats.find(name);
        if (it != stop_stats.end()) {
            const auto &route_set_proxy = it->second.route_set_proxy;
            if (!route_set_proxy) {
                it->second.InitProxy(name, route_stats);
            }
            auto buses = nlohmann::json::array();
            for (auto &bus: *route_set_proxy) {
                buses.emplace_back(bus);
            }
            return {
                    {"buses",      std::move(buses)},
                    {"request_id", request["id"]}
            };
        }
        return {
                {"request_id",    request["id"]},
                {"error_message", "not found"}
        };
    }


    void ProcessNewBusStop(const nlohmann::json &request) {
        stop_stats.emplace(
                request["name"],
                StopData{
                        Coordinates{request["latitude"], request["longitude"]},
                        std::nullopt,
                        request["road_distances"]
                }
        );
    }
};


#endif //BROWNFINAL_DATABASE_H
