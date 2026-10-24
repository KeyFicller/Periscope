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
// Properties applicable to a node: node-owned props, or base_object props shared by all objects
// (OP_name). Mirrors link_applicable_property for the node(graph) >> builder chain.
template<typename prop>
concept node_applicable_property = requires {
    typename prop::owner_type;
} && (std::is_base_of_v<node, typename prop::owner_type> || std::is_same_v<base_object, typename prop::owner_type>);
} // namespace detail

// Nested property enums whose owning property applies to node (e.g. NP_shape::shape)
template<typename e>
concept node_property_enum = property_enum<e> && detail::node_applicable_property<property_of_t<e>>;

template<typename lit>
concept node_property_literal =
  property_literal_value<lit> && detail::node_applicable_property<typename std::remove_cvref_t<lit>::property>;

// node_decoration lists what may sit in a `node(graph) >> …` builder chain.
// Same deduced forms as new_object<node> (UDL / nested enum / property_literal packs).
// Bare std::string / const char* / label{} are excluded (label{} is edge-only).
template<typename t>
concept node_decoration = node_property_enum<t> || node_property_literal<t>;

// node_builder is a typed incomplete node create used only as a temporary in a one-shot
// `node(graph) >> props…` expression. Distinct from pending_link / edge `>>` so node props
// do not steal edge syntax. Materializes to node& via conversion (calls new_object<node>,
// then apply_pack_arg for the accumulated pack). Incomplete builders must not be discarded:
// [[nodiscard]] + destructor abort if still live.
template<typename... props>
struct [[nodiscard("incomplete node(graph) builder: assign to node& (or cast) to materialize")]] node_builder
{
    void* Graph = nullptr;
    node& (*CreateFn)(void*) = nullptr;
    std::tuple<props...> Props{};

    node_builder() = default;
    node_builder(void* _graph, node& (*_create_fn)(void*), std::tuple<props...> _props = {})
      : Graph(_graph)
      , CreateFn(_create_fn)
      , Props(std::move(_props))
    {
    }

    node_builder(node_builder&& other) noexcept
      : Graph(std::exchange(other.Graph, nullptr))
      , CreateFn(std::exchange(other.CreateFn, nullptr))
      , Props(std::move(other.Props))
    {
    }
    node_builder& operator=(node_builder&& other) noexcept
    {
        if (this != &other) {
            Graph = std::exchange(other.Graph, nullptr);
            CreateFn = std::exchange(other.CreateFn, nullptr);
            Props = std::move(other.Props);
        }
        return *this;
    }
    node_builder(const node_builder&) = delete;
    node_builder& operator=(const node_builder&) = delete;

    // Materialize into the owning graph (same pack rules as new_object<node>).
    // Rvalue-only so a named builder cannot be converted twice.
    operator node&() &&
    {
        void* g = std::exchange(Graph, nullptr);
        auto* create = std::exchange(CreateFn, nullptr);
        auto pack = std::move(Props);
        node& n = create(g);
        std::apply([&n](auto&&... _p) { (n.apply_pack_arg(std::forward<decltype(_p)>(_p)), ...); }, std::move(pack));
        return n;
    }

    // A non-null Graph here means the builder never materialized (silent no-op would drop props).
    // Abort (not throw): safe if destruction runs during another exception unwind.
    ~node_builder()
    {
        if (Graph == nullptr)
            return;
        std::fputs("periscope: incomplete node(graph) builder — discarded without materializing to node& "
                   "(write `node& n = node(g) >> props`, not `node(g) >> props`)\n",
                   stderr);
        std::abort();
    }

    void release() noexcept
    {
        Graph = nullptr;
        CreateFn = nullptr;
    }
};

// ---- growing a node builder pack (typed; any deduced node property / UDL) ----

template<typename... props, typename deco>
    requires node_decoration<deco>
auto
operator>>(node_builder<props...>&& _builder, deco _decoration)
{
    // Reject duplicate *replace-strategy* owning properties at the append site (vs. silent
    // overwrite at set). Same helper as new_object / edge >>. Append / bitwise_or may repeat.
    static_assert(detail::deduced_args_unique_v<props..., deco>,
                  "periscope: duplicate replace-strategy owning property in node(graph) >> chain");
    using normalized = std::remove_cvref_t<deco>;
    void* g = _builder.Graph;
    auto* create = _builder.CreateFn;
    auto pack = std::move(_builder.Props);
    _builder.release();
    return node_builder<props..., normalized>{
        g, create, std::tuple_cat(std::move(pack), std::make_tuple(std::move(_decoration)))
    };
}

// ---- node(graph&) create-intermediary (constructor entry; type name stays `node`) ----

template<typename underlying_type>
node::node(graph<underlying_type>& _graph) noexcept
  : m_create_graph(&_graph)
  , m_create_fn(+[](void* p) -> node& {
        return static_cast<graph<underlying_type>*>(p)->template new_object<node>();
    })
{
    // Hollow temporary — not OP_printable, not inserted; first `>>` starts node_builder.
}

// Rvalue node >> deco starts the typed builder (lvalue node& >> deco remains edge start).
template<typename deco>
    requires node_decoration<deco>
auto
operator>>(node&& _n, deco _decoration)
{
    if (!_n.is_create_seed()) {
        std::fputs("periscope: node&& >> prop — not a create-intermediary (use node(graph) >> props)\n",
                   stderr);
        std::abort();
    }
    auto seed = _n.extract_create();
    using normalized = std::remove_cvref_t<deco>;
    static_assert(detail::deduced_args_unique_v<deco>,
                  "periscope: duplicate replace-strategy owning property in node(graph) >> chain");
    return node_builder<normalized>{ seed.Graph, seed.CreateFn, std::make_tuple(std::move(_decoration)) };
}
} // namespace periscope
