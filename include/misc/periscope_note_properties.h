#pragma once

#include "object/periscope_handle.h"
#include "object/periscope_object_properties.h"

namespace periscope {
// note is a class for notes
class note;

struct MNP_orient : public base_property<enum_type, note>
{
    enum orient : enum_type
    {
        k_left,
        k_right
    };

    // to_string is to convert property to string representation
    static std::string to_string(const MNP_orient& _property, graph_type _graph_type = io().GraphType)
    {
        if (_graph_type == graph_type::k_sequence) {
            return _property.Value == orient::k_left ? "left" : "right";
        }
        return "";
    }

    // to_string_default is to get default string representation
    static std::string to_string_default(graph_type _graph_type = io().GraphType)
    {
        static MNP_orient orient{ orient::k_left };
        return to_string(orient, _graph_type);
    }
};

struct MNP_basis : public base_property<std::vector<std::shared_ptr<base_handle>>, note>
{
    // to_string is to convert property to string representation
    static std::string to_string(const MNP_basis& _property, graph_type _graph_type = io().GraphType)
    {
        std::string str = "";
        if (_graph_type == graph_type::k_sequence) {
            for (const auto& handle : _property.Value) {
                str += handle->print();
                str += ", ";
            }
            str.erase(str.size() - 2, str.size());
        }
        return str;
    }
};
} // namespace periscope