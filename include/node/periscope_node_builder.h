#pragma once

// Included from the end of periscope_graph.h after class graph is complete.
// Do not include this header directly — pull periscope.h / periscope_graph.h.

#include "node/periscope_node.h"
#include "object/periscope_object_properties.h"
#include "type_traits/periscope_type_list.h"
#include <any>
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <type_traits>
#include <utility>

namespace periscope {
template<typename underlying_type, typename... props>
struct node_builder;

namespace detail {
// Properties applicable to a node: node-owned props, or base_object props shared by all objects
// (OP_name). Mirrors link_applicable_property for the node(graph) >> builder chain.
template<typename prop>
concept node_applicable_property = requires {
    typename prop::owner_type;
} && (std::is_base_of_v<node, typename prop::owner_type> || std::is_same_v<base_object, typename prop::owner_type>);

// Typed create-intermediary state stashed in node::m_create by node(graph&).
// Fluent node(graph) supports default graph<> (int handles) so >> can keep a single return type.
template<typename underlying_type>
struct node_create_bridge
{
    graph<underlying_type>* Graph = nullptr;

    template<typename deco>
    auto append(deco _decoration)
    {
        return node_builder<underlying_type>{ Graph } >> std::move(_decoration);
    }
};
} // namespace detail

// Nested property enums whose owning property applies to node (e.g. NP_shape::shape)
template<typename e>
concept node_property_enum = property_enum<e> && detail::node_applicable_property<property_of_t<e>>;

template<typename lit>
concept node_property_literal =
  property_literal_value<lit> && detail::node_applicable_property<typename std::remove_cvref_t<lit>::property>;

// node_decoration lists what may sit in a `node(graph) >> …` builder chain.
template<typename t>
concept node_decoration = node_property_enum<t> || node_property_literal<t>;

template<typename underlying_type, typename... props>
struct [[nodiscard("incomplete node(graph) builder: assign to node& (or cast) to materialize")]] node_builder
{
    graph<underlying_type>* Graph = nullptr;
    std::tuple<props...> Props{};

    node_builder() = default;
    explicit node_builder(graph<underlying_type>* _graph)
      : Graph(_graph)
    {
    }
    node_builder(graph<underlying_type>* _graph, std::tuple<props...> _props)
      : Graph(_graph)
      , Props(std::move(_props))
    {
    }

    node_builder(node_builder&& other) noexcept
      : Graph(std::exchange(other.Graph, nullptr))
      , Props(std::move(other.Props))
    {
    }
    node_builder& operator=(node_builder&& other) noexcept
    {
        if (this != &other) {
            Graph = std::exchange(other.Graph, nullptr);
            Props = std::move(other.Props);
        }
        return *this;
    }
    node_builder(const node_builder&) = delete;
    node_builder& operator=(const node_builder&) = delete;

    operator node&() &&
    {
        graph<underlying_type>* g = std::exchange(Graph, nullptr);
        auto pack = std::move(Props);
        return std::apply(
          [g](auto&&... _p) -> node& { return g->template new_object<node>(std::forward<decltype(_p)>(_p)...); },
          std::move(pack));
    }

    ~node_builder()
    {
        if (Graph == nullptr)
            return;
        std::fputs("periscope: incomplete node(graph) builder — discarded without materializing to node& "
                   "(write `node& n = node(g) >> props`, not `node(g) >> props`)\n",
                   stderr);
        std::abort();
    }

    void release() noexcept { Graph = nullptr; }
};

template<typename underlying_type, typename... props, typename deco>
    requires node_decoration<deco>
auto
operator>>(node_builder<underlying_type, props...>&& _builder, deco _decoration)
{
    static_assert(detail::deduced_args_unique_v<props..., deco>,
                  "periscope: duplicate replace-strategy owning property in node(graph) >> chain");
    using normalized = std::remove_cvref_t<deco>;
    graph<underlying_type>* g = _builder.Graph;
    auto pack = std::move(_builder.Props);
    _builder.release();
    return node_builder<underlying_type, props..., normalized>{
        g, std::tuple_cat(std::move(pack), std::make_tuple(std::move(_decoration)))
    };
}

// ---- node(graph&) create-intermediary (constructor entry; type name stays `node`) ----

template<typename underlying_type>
node::node(graph<underlying_type>& _graph)
{
    static_assert(std::is_same_v<underlying_type, int>,
                  "periscope: node(graph) fluent create supports graph<> (int handles) only; "
                  "use new_object<node>(...) for other handle types");
    m_create = detail::node_create_bridge<int>{ &_graph };
}

template<typename deco>
    requires node_decoration<deco>
auto
operator>>(node&& _n, deco _decoration)
{
    std::any bridge = _n.take_create();
    if (!bridge.has_value()) {
        std::fputs("periscope: node&& >> prop — not a create-intermediary (use node(graph) >> props)\n",
                   stderr);
        std::abort();
    }
    auto* b = std::any_cast<detail::node_create_bridge<int>>(&bridge);
    if (b == nullptr) {
        std::fputs("periscope: node(graph) create — internal bridge type mismatch\n", stderr);
        std::abort();
    }
    return b->append(std::move(_decoration));
}
} // namespace periscope
