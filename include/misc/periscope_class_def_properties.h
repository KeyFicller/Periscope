#pragma once

#include "object/periscope_object_properties.h"

namespace periscope {

class class_def;

// MCD_fill is the fill color of the class
struct MCD_fill : public base_property<std::string, class_def>
{
    // to_string is to convert property to string representation
    static std::string to_string(const MCD_fill& _property, graph_type _graph_type = io().GraphType)
    {
        return "fill:" + _property.Value + ",";
    }
};

// MCD_stroke is the stroke color of the class
struct MCD_stroke : public base_property<std::string, class_def>
{
    // to_string is to convert property to string representation
    static std::string to_string(const MCD_stroke& _property, graph_type _graph_type = io().GraphType)
    {
        return "stroke:" + _property.Value + ",";
    }
};

// MCD_color is the color of the class
struct MCD_color : public base_property<std::string, class_def>
{
    // to_string is to convert property to string representation
    static std::string to_string(const MCD_color& _property, graph_type _graph_type = io().GraphType)
    {
        return "color:" + _property.Value + ",";
    }
};
}