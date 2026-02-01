#pragma once

#include "object/periscope_handle.h"
#include "object/periscope_object_properties.h"

namespace periscope {
// ------------------------ Main template -----------------------

class link;

// LP_source is property for link source handle
struct LP_source : public base_property<std::shared_ptr<base_handle>, link>
{
    using owner_type = link;
};

// LP_target is property for link target handle
struct LP_target : public base_property<std::shared_ptr<base_handle>, link>
{
    using owner_type = link;
};

// LP_style is property for link line style
struct LP_style : public base_property<enum_type, link>
{

    enum style : enum_type
    {
        k_solid,
        k_dashed,
        k_bold_solid,

        k_arrow_mask = 0x100000,
    };

    // to_string is to convert property to string representation
    static std::string to_string(const LP_style& _property, graph_type _graph_type)
    {
        if (_graph_type != graph_type::k_flowchart)
            return "";
        switch (_property.Value & ~style::k_arrow_mask) {
            case style::k_solid:
                return std::string("--") + (_property.Value & style::k_arrow_mask ? ">" : "");
            case style::k_dashed:
                return std::string("-.-") + (_property.Value & style::k_arrow_mask ? ">" : "");
            case style::k_bold_solid:
                return "==>";
            default:
                throw std::runtime_error("Unsupported style");
        }
    }

    // to_string_default is to get default string representation
    static std::string to_string_default(graph_type _graph_type)
    {
        static LP_style style{ style::k_solid | style::k_arrow_mask };
        return to_string(style, _graph_type);
    }
};
}