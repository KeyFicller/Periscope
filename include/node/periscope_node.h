#pragma once

#include "node/periscope_node_properties.h"
#include "object/periscope_create_seed.h"
#include "object/periscope_object_properties.h"
#include <format>

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
// Create-seed state / move / incomplete abort live in create_seed<node> (shared with note /
// class_def); link stays on plain object<link>.
class node : public create_seed<node>
{
  public:
    node() { set<OP_printable>(true); }

    // Create-intermediary entry (defined after graph<> is complete in periscope_object_builder.h).
    template<typename underlying_type>
    explicit node(graph<underlying_type>& _graph) noexcept;

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
};
} // namespace periscope
