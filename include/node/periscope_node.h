#pragma once

#include "node/periscope_node_properties.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"
#include <cstdio>
#include <cstdlib>
#include <format>
#include <utility>

namespace periscope {
// ------------------------ Main template -----------------------

// Forward-declared in periscope_handle.h (default underlying_type = int).
template<typename underlying_type>
class graph;

// node is the graph-node object type.
// Create spelling: `node& n = node(graph) >> props…` — the graph& constructor makes a
// create-intermediary temporary (not yet in the graph); rvalue `>>` grows a typed
// object_builder<node, …> pack; conversion to node& materializes via new_object<node>.
// Lvalue `node&` keeps one-shot edge `>>` (see periscope_link_operators.h).
class node : public object<node>
{
  public:
    node() { set<OP_printable>(true); }

    // Create-intermediary entry (defined after graph<> is complete in periscope_object_builder.h).
    template<typename underlying_type>
    explicit node(graph<underlying_type>& _graph) noexcept;

    node(const node&) = delete;
    node& operator=(const node&) = delete;

    node(node&& other) noexcept
      : object<node>(static_cast<object<node>&&>(other))
      , m_create_graph(std::exchange(other.m_create_graph, nullptr))
      , m_create_fn(std::exchange(other.m_create_fn, nullptr))
    {
    }

    node& operator=(node&& other) noexcept
    {
        if (this != &other) {
            static_cast<object<node>&>(*this) = static_cast<object<node>&&>(other);
            m_create_graph = std::exchange(other.m_create_graph, nullptr);
            m_create_fn = std::exchange(other.m_create_fn, nullptr);
        }
        return *this;
    }

    ~node()
    {
        if (m_create_fn == nullptr)
            return;
        std::fputs("periscope: incomplete node(graph) — discarded without materializing to node& "
                   "(write `node& n = node(g) >> props`, not `node(g) >> props`)\n",
                   stderr);
        std::abort();
    }

    // Create-seed helpers for object_builder / operator>>(node&&, …).
    bool is_create_seed() const noexcept { return m_create_fn != nullptr; }

    struct create_seed_state
    {
        void* Graph = nullptr;
        node& (*CreateFn)(void*) = nullptr;
    };

    create_seed_state extract_create() noexcept
    {
        return { std::exchange(m_create_graph, nullptr), std::exchange(m_create_fn, nullptr) };
    }

  public:
    // to_string_impl is implementation of object::to_string_impl
    std::string to_string_impl(graph_type _graph_type = io().GraphType) const
    {
        switch (_graph_type) {
            case graph_type::k_flowchart: {
                if (has<NP_subgraph_node>()) {
                    std::string subgraph_str = "subgraph ";
                    subgraph_str += get_handle()->print();
                    subgraph_str += std::format("[\"{}\"]", _V_str<OP_name>());
                    subgraph_str += "\n";
                    for (const auto& handle : get<NP_subgraph_node>().Value) {
                        subgraph_str += handle->print();
                        subgraph_str += "\n";
                    }
                    subgraph_str += "end\n";
                    return subgraph_str;
                } else {
                    std::string handle_str = get_handle()->print();
                    std::string shape_str = _V_str<NP_shape>();
                    std::string label_str = get<OP_name>().Value;
                    return std::format("{}@{{ shape: {}, label: {} }}", handle_str, shape_str, label_str);
                }
            }
            case graph_type::k_sequence: {
                std::string handle_str = get_handle()->print();
                std::string label_str = get<OP_name>().Value;
                return std::format("participant {} as {}", handle_str, label_str);
            }
            case graph_type::k_gantt: {
                std::string label_str = get<OP_name>().Value;
                std::string start_tick_str = _V_str<NP_time_tick_start>();
                std::string end_tick_str = _V_str<NP_time_tick_end>();
                return std::format("    {} : {}, {}", label_str, start_tick_str, end_tick_str);
            }
            default:
                throw std::runtime_error("Unsupported graph type");
        }
    }

  private:
    // Create-intermediary only (null for graph-owned nodes): target graph + typed new_object trampoline.
    void* m_create_graph = nullptr;
    node& (*m_create_fn)(void*) = nullptr;
};
} // namespace periscope
