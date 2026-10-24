#pragma once

// Included from the end of periscope_graph.h after class graph is complete.
// Do not include this header directly — pull periscope.h / periscope_graph.h.

#include "node/periscope_node.h"
#include "object/periscope_object_properties.h"
#include "type_traits/periscope_type_list.h"
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <type_traits>
#include <utility>

namespace periscope {
namespace detail {
// Properties applicable to a Node: Node-owned props, or base_object props shared by all objects
// (OP_name). Mirrors link_applicable_property for the node(...) >> builder chain.
template<typename prop>
concept node_applicable_property = requires {
    typename prop::owner_type;
} && (std::is_base_of_v<Node, typename prop::owner_type> || std::is_same_v<base_object, typename prop::owner_type>);
} // namespace detail

// Nested property enums whose owning property applies to Node (e.g. NP_shape::shape)
template<typename e>
concept node_property_enum = property_enum<e> && detail::node_applicable_property<property_of_t<e>>;

template<typename lit>
concept node_property_literal =
  property_literal_value<lit> && detail::node_applicable_property<typename std::remove_cvref_t<lit>::property>;

// node_decoration lists what may sit in a `node(graph) >> …` builder chain.
// Same deduced forms as new_object<Node> (UDL / nested enum / property_literal packs).
// Bare std::string / const char* / label{} are excluded (label{} is edge-only).
template<typename t>
concept node_decoration = node_property_enum<t> || node_property_literal<t>;

// node_builder is a typed incomplete Node create used only as a temporary in a one-shot
// `node(graph) >> props…` expression. Distinct from pending_link / edge `>>` so Node props
// do not steal edge syntax. Materializes to Node& via conversion (calls new_object<Node>).
// Incomplete builders must not be discarded: [[nodiscard]] + destructor abort if still live.
template<typename underlying_type, typename... props>
struct [[nodiscard("incomplete node(graph) builder: assign to Node& (or cast) to materialize")]] node_builder
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

    // Materialize into the owning graph (same pack rules as new_object<Node>).
    // Rvalue-only so a named builder cannot be converted twice.
    operator Node&() &&
    {
        graph<underlying_type>* g = std::exchange(Graph, nullptr);
        auto pack = std::move(Props);
        return std::apply(
          [g](auto&&... _p) -> Node& { return g->template new_object<Node>(std::forward<decltype(_p)>(_p)...); },
          std::move(pack));
    }

    // A non-null Graph here means the builder never materialized (silent no-op would drop props).
    // Abort (not throw): safe if destruction runs during another exception unwind.
    ~node_builder()
    {
        if (Graph == nullptr)
            return;
        std::fputs("periscope: incomplete node(graph) builder — discarded without materializing to Node& "
                   "(write `Node& n = node(g) >> props`, not `node(g) >> props`)\n",
                   stderr);
        std::abort();
    }

    void release() noexcept { Graph = nullptr; }
};

// ---- growing a node builder pack (typed; any deduced Node property / UDL) ----

template<typename underlying_type, typename... props, typename deco>
    requires node_decoration<deco>
auto
operator>>(node_builder<underlying_type, props...>&& _builder, deco _decoration)
{
    // Reject duplicate *replace-strategy* owning properties at the append site (vs. silent
    // overwrite at set). Same helper as new_object / edge >>. Append / bitwise_or may repeat.
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

// Primary create spelling: `Node& n = node(_graph) >> "Name"_op_name >> NP_shape::k_rectangle;`
// class Node was renamed from `node` so this constexpr function object can own the name `node`
// in namespace periscope (a free function `node(graph&)` next to `class node` cannot).
struct node_fn
{
    template<typename underlying_type>
    [[nodiscard]] node_builder<underlying_type> operator()(graph<underlying_type>& _graph) const
    {
        return node_builder<underlying_type>{ &_graph };
    }
};
inline constexpr node_fn node{};
} // namespace periscope
