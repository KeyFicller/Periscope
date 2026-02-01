#pragma once

#include "misc/periscope_note_properties.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"
#include <format>

namespace periscope {
// note is a class for notes
class note : public object<note>
{
  public:
    note() { set<OP_printable>(true); }

  public:
    // to_string_impl is implementation of object::to_string_impl
    std::string to_string_impl(graph_type _graph_type = io().GraphType) const
    {
        if (!has<MNP_basis>())
            return "";

        auto& basis = get<MNP_basis>().Value;
        if (_graph_type == graph_type::k_sequence) {
            if (basis.size() != 1) {
                return std::format("Note over {} : {}", _V_str<MNP_basis>(), _V_str<OP_name>());
            } else {
                return std::format("Note {} of {} : {}", _V_str<MNP_orient>(), _V_str<MNP_basis>(), _V_str<OP_name>());
            }
        }

        return "";
    }
};
} // namespace periscope