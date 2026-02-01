#pragma once

#include "misc/periscope_class_def_properties.h"
#include "object/periscope_object.h"

namespace periscope {

// class_def is a class definition object
class class_def : public object<class_def>
{
  public:
    class_def() { set<OP_printable>(true); }

    // to_string_impl is implementation of object::to_string_impl
    std::string to_string_impl(graph_type _graph_type = io().GraphType) const
    {
        if (_graph_type == graph_type::k_flowchart) {
            if (!has<OP_name>()) {
                throw std::runtime_error("class_def: name is required");
            }
            std::string props = _V_str<MCD_fill>() + _V_str<MCD_stroke>() + _V_str<MCD_color>();
            if (props.back() == ',') {
                props.pop_back();
            }
            return "classDef " + _V_str<OP_name>() + "Class" + " " + props;
        }
        return "";
    }
};

} // namespace periscope