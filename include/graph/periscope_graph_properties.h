#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_object_properties.h"

namespace periscope {
// ------------------------ Main template -----------------------

template<typename underlying_type>
class graph;

// GP_type is forward declaration for graph type property
template<typename underlying_type>
struct GP_type;

// GP_display_node is property for whether to display nodes
template<typename underlying_type>
struct GP_display_node : base_property<bool, graph<underlying_type>>
{
    using parent_properties = type_list<GP_type<underlying_type>>;
    static std::string to_string(const GP_display_node& _property, graph_type _graph_type)
    {
        switch (_graph_type) {
            case graph_type::k_sequence:
                return _property.template base_property<bool, graph<underlying_type>>::Value ? "autonumber" : "";
            default:
                return "";
        }
    }
};

// GP_type is property for graph type
template<typename underlying_type>
struct GP_type : base_property<graph_type, graph<underlying_type>>
{
    using child_properties = type_list<GP_display_node<underlying_type>>;

    static std::string to_string(const GP_type& _property, graph_type)
    {
        switch (_property.Value) {
            case graph_type::k_flowchart:
                return "flowchart";
            case graph_type::k_sequence:
                return "sequenceDiagram";
            default:
                return "";
        }
    }
};

// GP_output_format is property for graph output format
template<typename underlying_type>
struct GP_output_format : base_property<graph_output_format, graph<underlying_type>>
{};
}