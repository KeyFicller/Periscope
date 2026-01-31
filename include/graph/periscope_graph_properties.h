#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_object_properties.h"

namespace periscope {

template<typename underlying_type>
class graph;

template<typename underlying_type>
struct GP_type;

template<typename underlying_type>
struct GP_display_node : base_property<bool, graph<underlying_type>>
{
    using parent_properties = type_list<GP_type<underlying_type>>;
};

template<typename underlying_type>
struct GP_type : base_property<graph_type, graph<underlying_type>>
{
    using child_properties = type_list<GP_display_node<underlying_type>>;
};

template<typename underlying_type>
struct GP_output_format : base_property<graph_output_format, graph<underlying_type>>
{};
}