#include "Requests.cpp"

struct Items{
    double time;
    Request::Name type;
    std::optional<std::string_view> bus = std::nullopt;
    std::optional<int> span_count = std::nullopt;
};
namespace Graph {

    template <typename Weight>
    class Router {
    private:
        using Graph = DirectedWeightedGraph<Weight>;

    public:
        Router(std::shared_ptr<Graph> graph);
        Router(const Router& g):
                expanded_routes_cache_(g.expanded_routes_cache_),
                graph_(g.graph_), next_route_id_(g.next_route_id_),
                routes_internal_data_(g.routes_internal_data_)
        {};
        Router(Router&& g):
                expanded_routes_cache_(std::move(g.expanded_routes_cache_)),
                graph_(g.graph_), next_route_id_(std::move(g.next_route_id_)),
                routes_internal_data_(std::move(g.routes_internal_data_))
        {};

        using RouteId = uint64_t;


        struct RouteInfo {
            RouteId id;
            Weight weight;
            std::vector<Items> it;
        };

        std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to, double wait_time) const;
        EdgeId GetRouteEdge(RouteId route_id, size_t edge_idx) const;
        void ReleaseRoute(RouteId route_id);
        std::shared_ptr<Graph> graph_;
    private:


        struct RouteInternalData {
            Weight weight;
            std::optional<EdgeId> prev_edge;
        };
        using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

        using ExpandedRoute = std::vector<EdgeId>;
        mutable RouteId next_route_id_ = 0;
        mutable std::unordered_map<RouteId, ExpandedRoute> expanded_routes_cache_;

        void InitializeRoutesInternalData(std::shared_ptr<Graph> graph) {
            const size_t vertex_count = graph->GetVertexCount();
            for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
                routes_internal_data_[vertex][vertex] = RouteInternalData{0, std::nullopt};
                for (const EdgeId edge_id : graph->GetIncidentEdges(vertex)) {
                    const auto& edge = graph->GetEdge(edge_id);
                    auto& route_internal_data = routes_internal_data_[vertex][edge.to];
                    if (!route_internal_data || route_internal_data->weight > edge.weight) {
                        route_internal_data = RouteInternalData{edge.weight, edge_id};
                    }
                }
            }
        }

        void RelaxRoute(VertexId vertex_from, VertexId vertex_to,
                        const RouteInternalData& route_from, const RouteInternalData& route_to) {
            auto& route_relaxing = routes_internal_data_[vertex_from][vertex_to];
            const Weight candidate_weight = route_from.weight + route_to.weight;
            if (!route_relaxing || candidate_weight < route_relaxing->weight) {
                route_relaxing = {
                        candidate_weight,
                        route_to.prev_edge
                        ? route_to.prev_edge
                        : route_from.prev_edge
                };
            }
        }

        void RelaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) {
            for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
                if (const auto& route_from = routes_internal_data_[vertex_from][vertex_through]) {
                    for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) {
                        if (const auto& route_to = routes_internal_data_[vertex_through][vertex_to]) {
                            RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                        }
                    }
                }
            }
        }
        RoutesInternalData routes_internal_data_;
    };

    template <typename Weight>
    Router<Weight>::Router(std::shared_ptr<Graph> graph)
            : graph_(graph),
              routes_internal_data_(graph->GetVertexCount(), std::vector<std::optional<RouteInternalData>>(graph->GetVertexCount()))
    {
        InitializeRoutesInternalData(graph);

        const size_t vertex_count = graph->GetVertexCount();
        for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
            RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
        }
    }

    template <typename Weight>
    std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from, VertexId to, double wait_time) const {
        const RouteId route_id = next_route_id_++;
        std::vector<Items> v_it;
        if(from == to){
            return RouteInfo{route_id, 0.0, std::move(v_it)};
        }
        const auto& route_internal_data = routes_internal_data_[from][to];
        if (!route_internal_data) {
            return std::nullopt;
        }
        const Weight weight = route_internal_data->weight;
        std::optional<EdgeId> edge_id;
        for (edge_id = route_internal_data->prev_edge;edge_id;
             edge_id = routes_internal_data_[from][graph_->GetEdge(*edge_id).from]->prev_edge){
            v_it.push_back(Items{graph_->GetEdge(edge_id.value()).weight - wait_time, Request::Name::BUS,
                                 std::optional(graph_->GetEdge(edge_id.value()).bus), std::optional(graph_->GetEdge(edge_id.value()).span_count)});
            v_it.push_back(Items{wait_time, Request::Name::WAIT,
                                 std::optional(graph_->GetEdge(edge_id.value()).stop), std::nullopt});

        }
        return RouteInfo{route_id, weight, std::move(v_it)};
    }



}
