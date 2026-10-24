#pragma once

#include "misc/periscope_class_def_properties.h"
#include "object/periscope_create_seed.h"
#include <stdexcept>
#include <string>

namespace periscope {

// Forward-declared in periscope_handle.h (default underlying_type = int).
template<typename underlying_type>
class graph;

// class_def is a class definition object.
// Create spelling: `class_def& c = class_def(graph) >> props…` — same create-intermediary
// shape as node (graph& ctor → object_builder<class_def, …> → new_object<class_def>).
// new_object<class_def> still works.
// Create-seed state lives in create_seed<class_def> (shared with node / note).
class class_def : public create_seed<class_def>
{
  public:
    class_def() { set<OP_printable>(true); }

    // Create-intermediary entry (defined after graph<> is complete in periscope_object_builder.h).
    template<typename underlying_type>
    explicit class_def(graph<underlying_type>& _graph) noexcept;

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
