#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_object_properties.h"

namespace periscope {
// ------------------------ Main template -----------------------

class node;

// NP_shape is property for node shape
struct NP_shape : base_property<enum_type, node>
{
    enum shape : enum_type
    {
        k_circle,
        k_rectangle,
    };

    // to_string is to convert property to string representation
    static std::string to_string(const NP_shape& _property, graph_type _graph_type)
    {
        switch (_graph_type) {
            case graph_type::k_flowchart:
                return _property.template base_property<enum_type, node>::Value == k_rectangle ? "rect" : "circle";
            default:
                throw std::runtime_error("Unsupported graph type");
        }
    }
};
}