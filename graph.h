template <typename It>
class Range {
public:
    using ValueType = typename std::iterator_traits<It>::value_type;

    Range(It begin, It end) : begin_(begin), end_(end) {}
    It begin() const { return begin_; }
    It end() const { return end_; }

private:
    It begin_;
    It end_;
};


namespace Graph {

    using VertexId = size_t;
    using EdgeId = size_t;

    template <typename Weight>
    struct Edge {
        VertexId from;
        VertexId to;
        Weight weight;
        size_t span_count;
        std::optional<std::string_view> bus = std::nullopt;
        std::optional<std::string_view> stop = std::nullopt;
    };

    template <typename Weight>
    class DirectedWeightedGraph {
    private:
        using IncidenceList = std::vector<EdgeId>;
        using IncidentEdgesRange = Range<typename IncidenceList::const_iterator>;

    public:
        DirectedWeightedGraph(size_t vertex_count);
        DirectedWeightedGraph(const DirectedWeightedGraph<Weight>& g):edges_(g.edges_), incidence_lists_(g.incidence_lists_) {};
        DirectedWeightedGraph(DirectedWeightedGraph<Weight>&& g):edges_(std::move(g.edges_)), incidence_lists_(std::move(g.incidence_lists_)) {};
        EdgeId AddEdge(const Edge<Weight>& edge);

        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;
        const Edge<Weight>& GetEdge(EdgeId edge_id) const;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

    public:
        std::vector<Edge<Weight>> edges_;
        std::vector<IncidenceList> incidence_lists_;
    };


    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count) :
            incidence_lists_(std::vector<IncidenceList>(vertex_count, IncidenceList())) {}

    template <typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
        edges_.push_back(edge);
        const EdgeId id = edges_.size() - 1;
        incidence_lists_[edge.from].push_back(id);
        return id;
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
        return incidence_lists_.size();
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
        return edges_.size();
    }

    template <typename Weight>
    const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
        return edges_[edge_id];
    }

    template <typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
    DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
        const auto& edges = incidence_lists_[vertex];
        return {std::begin(edges), std::end(edges)};
    }
}
