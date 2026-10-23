#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_object_properties.h"
#include <format>

namespace periscope {
// ------------------------ Main template -----------------------

template<typename underlying_type>
class graph;

// graph_property_tag carries a graph property template so graph<u> can rebind it to its own handle type
template<template<typename> class prop>
struct graph_property_tag
{};

template<typename t>
struct is_graph_property_tag : std::false_type
{};

template<template<typename> class prop>
struct is_graph_property_tag<graph_property_tag<prop>> : std::true_type
{};

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

// GP_type is property for graph type; chart_type lives once in GP_type_base
template<typename underlying_type = int>
struct GP_type
  : base_property<enum_type, graph<underlying_type>>
  , GP_type_base
{
    using child_properties = type_list<GP_sequence_show_number<underlying_type>>;

    static std::string to_string(const GP_type& _property, graph_type _graph_type = io().GraphType)
    {
        (void)_graph_type;
        switch (_property.Value) {
            case chart_type::k_flowchart:
                return "flowchart";
            case chart_type::k_sequence:
                return "sequenceDiagram";
            case chart_type::k_gantt:
                return "gantt";
            default:
                return "";
        }
    }
};

// Free function (not a hidden friend of GP_type): chart_type is nested in GP_type_base,
// so ADL only associates GP_type_base / namespace periscope — not the template.
inline graph_property_tag<GP_type>
periscope_property_of(GP_type_base::chart_type)
{
    return {};
}

// GP_output_format is property for output format; output_format lives once in GP_output_format_base
template<typename underlying_type = int>
struct GP_output_format
  : base_property<enum_type, graph<underlying_type>>
  , GP_output_format_base
{};

inline graph_property_tag<GP_output_format>
periscope_property_of(GP_output_format_base::output_format)
{
    return {};
}

// GP_flowchart_direction is property for graph flowchart direction
template<typename underlying_type = int>
struct GP_flowchart_direction : base_property<enum_type, graph<underlying_type>>
{
    enum flowchart_direction : enum_type
    {
        k_top_to_down,
        k_down_to_top,
        k_left_to_right,
        k_right_to_left,
    };
    friend graph_property_tag<GP_flowchart_direction> periscope_property_of(flowchart_direction) { return {}; }

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

// GP_date_format is property for gantt chart Mermaid dateFormat
template<typename underlying_type = int>
struct GP_date_format : base_property<enum_type, graph<underlying_type>>
{
    enum date_format : enum_type
    {
        k_month_day,
        k_hour_minute
    };
    friend graph_property_tag<GP_date_format> periscope_property_of(date_format) { return {}; }

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

    // format_tick zero-pads a data_tick to match Mermaid dateFormat (HH:mm / MM-DD)
    static std::string format_tick(int first, int second, enum_type fmt = date_format::k_hour_minute)
    {
        switch (fmt) {
            case date_format::k_month_day:
                return std::format("{:02}-{:02}", first, second);
            case date_format::k_hour_minute:
            default:
                return std::format("{:02}:{:02}", first, second);
        }
    }

    static std::string to_string_default(graph_type _graph_type = io().GraphType)
    {
        if (_graph_type != graph_type::k_gantt)
            return "";
        static GP_date_format<underlying_type> default_date_format_property;
        default_date_format_property.Value = date_format::k_hour_minute;
        return to_string(default_date_format_property, _graph_type);
    }
};
}
