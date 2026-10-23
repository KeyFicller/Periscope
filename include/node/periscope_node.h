#pragma once

#include "misc/periscope_class_def.h"
#include "node/periscope_node_properties.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"
#include <format>

namespace periscope {
// ------------------------ Main template -----------------------

// node is graph node object
class node : public object<node>
{
  public:
    node() { set<OP_printable>(true); }

    // Bring object<node>::set overloads into scope (non-template set below would hide them).
    using object<node>::set;

    // Typed NP_class_def bind: use the class_def object (its OP_name) instead of a
    // stringly-typed "Name"_np_class_def when the definition is in scope.
    node& set(const class_def& _class)
    {
        if (!_class.has<OP_name>())
            throw std::runtime_error("periscope: class_def used as NP_class_def requires OP_name");
        return set<NP_class_def>(_class.get<OP_name>().Value);
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
};
}