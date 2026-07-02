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

// GP_sequence_show_number is property for whether to show sequence number
template<typename underlying_type>
struct GP_sequence_show_number : base_property<bool, graph<underlying_type>>
{
    using parent_properties = type_list<GP_type<underlying_type>>;
    static std::string to_string(const GP_sequence_show_number& _property, graph_type _graph_type = io().GraphType)
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
    using child_properties = type_list<GP_sequence_show_number<underlying_type>>;

    static std::string to_string(const GP_type& _property, graph_type _graph_type = io().GraphType)
    {
        switch (_property.Value) {
            case graph_type::k_flowchart:
                return "flowchart";
            case graph_type::k_sequence:
                return "sequenceDiagram";
            case graph_type::k_gantt:
                return "gantt";
            default:
                return "";
        }
    }
};

// GP_output_format is property for graph output format
template<typename underlying_type>
struct GP_output_format : base_property<graph_output_format, graph<underlying_type>>
{};

// GP_flowchart_direction is property for graph flowchart direction
template<typename underlying_type>
struct GP_flowchart_direction : base_property<enum_type, graph<underlying_type>>
{
    enum flowchart_direction : enum_type
    {
        k_top_to_down,
        k_down_to_top,
        k_left_to_right,
        k_right_to_left,
    };

    static std::string to_string(const GP_flowchart_direction& _property, graph_type _graph_type = io().GraphType)
    {
        if (_graph_type != graph_type::k_flowchart)
            return "";
        switch (_property.Value) {
            case flowchart_direction::k_top_to_down:
                return "TD";
            case flowchart_direction::k_down_to_top:
                return "BT";
            case flowchart_direction::k_left_to_right:
                return "LR";
            case flowchart_direction::k_right_to_left:
                return "RL";
            default:
                throw std::runtime_error("Unsupported flowchart direction");
        }
    }
};

// GP_dateFormat is property for gantt chart date format
template<typename underlying_type>
struct GP_date_format : base_property<enum_type, graph<underlying_type>>
{
    enum date_format : enum_type
    {
        k_month_day,
        k_hour_minute
    };

    static std::string to_string(const GP_date_format& _property, graph_type _graph_type = io().GraphType)
    {
        if (_graph_type != graph_type::k_gantt)
            return "";
        switch (_property.Value) {
            case date_format::k_month_day:
                return "dateFormat MM-DD";
            case date_format::k_hour_minute:
                return "dateFormat HH:mm";
            default:
                return "";
        }
    }

    static std::string to_string_default(graph_type _graph_type = io().GraphType)
    {
        if (_graph_type != graph_type::k_gantt)
            return "";
        static GP_date_format<underlying_type> default_date_format_property;
        default_date_format_property.Value = GP_date_format<underlying_type>::date_format::k_hour_minute;
        return to_string(default_date_format_property, _graph_type);
    }
};
}