#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_handle.h"
#include "object/periscope_object_properties.h"
#include <vector>

namespace periscope {
// ------------------------ Main template -----------------------

class node;

// NP_shape is property for node shape
struct NP_shape : base_property<enum_type, node>
{
    enum shape : enum_type
    {
        k_rectangle,
        k_round_rectangle,
        k_diamond,
        k_parallelogram,
    };

    // to_string is to convert property to string representation
    static std::string to_string(const NP_shape& _property, graph_type _graph_type)
    {
        if (_graph_type != graph_type::k_flowchart)
            return "";
        switch (_property.Value) {
            case shape::k_rectangle:
                return "rect";
            case shape::k_round_rectangle:
                return "roundrect";
            case shape::k_diamond:
                return "diamond";
            case shape::k_parallelogram:
                return "parallelogram";
            default:
                throw std::runtime_error("Unsupported shape");
        }
    }

    // to_string_default is to get default string representation
    static std::string to_string_default(graph_type _graph_type)
    {
        static NP_shape shp{ shape::k_rectangle };
        return to_string(shp, _graph_type);
    }
};

// NP_subgraph is property for node subgraph
struct NP_subgraph_node : base_property<std::vector<std::shared_ptr<base_handle>>, node>
{};
} // namespace periscope