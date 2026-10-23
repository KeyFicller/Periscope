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

        // Composed styles — first-class nested enumerators so `>>` chains can
        // write `LP_style::k_dashed_arrow` instead of `k_dashed | k_arrow_mask`.
        k_solid_arrow = k_solid | k_arrow_mask,
        k_dashed_arrow = k_dashed | k_arrow_mask,
    };
    friend LP_style periscope_property_of(style) { return {}; }

    // operator| keeps composed styles typed, so `k_dashed | k_arrow_mask` still deduces LP_style
    friend constexpr style operator|(style _lhs, style _rhs)
    {
        return static_cast<style>(static_cast<enum_type>(_lhs) | static_cast<enum_type>(_rhs));
    }

    // to_string is to convert property to string representation
    static std::string to_string(const LP_style& _property, graph_type _graph_type = io().GraphType)
    {
        if (_graph_type == graph_type::k_flowchart) {
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
        } else if (_graph_type == graph_type::k_sequence) {
            switch (_property.Value & ~style::k_arrow_mask) {
                case style::k_solid:
                    return std::string("->") + (_property.Value & style::k_arrow_mask ? ">" : "");
                case style::k_dashed:
                    return std::string("-->") + (_property.Value & style::k_arrow_mask ? ">" : "");
            }
        }

        return "";
    }

    // to_string_default is to get default string representation
    static std::string to_string_default(graph_type _graph_type = io().GraphType)
    {
        static LP_style style{ style::k_solid_arrow };
        return to_string(style, _graph_type);
    }
};

// LP_activate is sequence activation (+/-); nested enum matches NP_shape / LP_style
struct LP_activate : public base_property<enum_type, link>
{
    enum activation : enum_type
    {
        k_deactivate = 0,
        k_activate = 1,
    };
    friend LP_activate periscope_property_of(activation) { return {}; }

    // to_string is to convert property to string representation
    static std::string to_string(const LP_activate& _property, graph_type _graph_type = io().GraphType)
    {
        (void)_graph_type;
        return _property.Value == activation::k_activate ? "+" : "-";
    }

    // to_string_default is to get default string representation (unset → no suffix)
    static std::string to_string_default(graph_type _graph_type = io().GraphType)
    {
        (void)_graph_type;
        return "";
    }
};
}
