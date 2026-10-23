#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_handle.h"
#include "object/periscope_object_properties.h"
#include <format>
#include <memory>
#include <string>
#include <string_view>
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
        k_diamond,
        k_parallelogram,
    };
    friend NP_shape periscope_property_of(shape) { return {}; }

    // to_string is to convert property to string representation
    static std::string to_string(const NP_shape& _property, graph_type _graph_type = io().GraphType)
    {
        if (_graph_type != graph_type::k_flowchart)
            return "";
        switch (_property.Value) {
            case shape::k_rectangle:
                return "rect";
            case shape::k_diamond:
                return "diamond";
            case shape::k_parallelogram:
                return "parallelogram";
            default:
                throw std::runtime_error("Unsupported shape");
        }
    }

    // to_string_default is to get default string representation
    static std::string to_string_default(graph_type _graph_type = io().GraphType)
    {
        static NP_shape shp{ shape::k_rectangle };
        return to_string(shp, _graph_type);
    }
};

// NP_subgraph is property for node subgraph
struct NP_subgraph_node : base_property<std::vector<std::shared_ptr<base_handle>>, node>
{};

// NP_class_def is property for node class definition
struct NP_class_def : base_property<std::string, node>
{
    // to_string is to convert property to string representation
    static std::string to_string(const NP_class_def& _property, graph_type _graph_type = io().GraphType)
    {
        (void)_graph_type;
        return _property.Value;
    }
};

// "Start"_np_class_def -> property_literal<NP_class_def> (NP_ = node_property)
inline property_literal<NP_class_def>
operator""_np_class_def(const char* _str, std::size_t _len)
{
    return property_literal<NP_class_def>{ std::string(_str, _len) };
}

// NP_time_tick is property for node time tick

// It's meaning depends on the graph type
struct data_tick
{
    int first;
    int second;
};

inline data_tick
operator""_tick(const char* _str, std::size_t _len)
{
    // Split by :
    std::string_view str_view(_str, _len);
    auto pos = str_view.find(':');
    if (pos == std::string_view::npos) {
        throw std::runtime_error("Invalid time tick format");
    }
    return data_tick{ std::stoi(std::string(_str, 0, pos)), std::stoi(std::string(_str, pos + 1, _len - pos - 1)) };
}

struct NP_time_tick : base_property<data_tick, node>
{
    // to_string is a raw tick dump; gantt Mermaid padding is done in graph::to_string_impl
    // via GP_date_format::format_tick (io() only holds graph-universal state).
    static std::string to_string(const NP_time_tick& _property, graph_type _graph_type = io().GraphType)
    {
        return std::format("{}:{}", _property.Value.first, _property.Value.second);
    }
};

struct NP_time_tick_start : public NP_time_tick
{
    using NP_time_tick::NP_time_tick;
};

struct NP_time_tick_end : public NP_time_tick
{
    using NP_time_tick::NP_time_tick;
};
} // namespace periscope