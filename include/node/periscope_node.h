#pragma once

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

  public:
    // to_string_impl is implementation of object::to_string_impl
    std::string to_string_impl(graph_type graph_type) const
    {
        switch (graph_type) {
            case graph_type::k_flowchart: {
                std::string handle_str = get_handle()->print(graph_type);
                std::string shape_str = get<NP_shape>().Value == NP_shape::k_rectangle ? "rect" : "circle";
                std::string label_str = get<OP_name>().Value;
                return std::format("{}@{{ shape: {}, label: {} }}", handle_str, shape_str, label_str);
            }
            case graph_type::k_sequence: {
                std::string handle_str = get_handle()->print(graph_type);
                std::string label_str = get<OP_name>().Value;
                return std::format("participant {} as {}", handle_str, label_str);
            }
            default:
                throw std::runtime_error("Unsupported graph type");
        }
    }
};
}