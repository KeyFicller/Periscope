#pragma once

#include "object/periscope_object_properties.h"

namespace periscope {

template<typename underlying_type>
class graph;

template<typename underlying_type>
struct GP_type : base_property<enum_type, graph<underlying_type>>
{
    enum type : enum_type
    {
        k_flowchart,
    };
};
}