#pragma once

#include "misc/periscope_note_properties.h"
#include "object/periscope_create_seed.h"
#include "object/periscope_object_properties.h"
#include <format>

namespace periscope {
// Forward-declared in periscope_handle.h (default underlying_type = int).
template<typename underlying_type>
class graph;

// note is a class for notes.
// Create spelling: `note& n = note(graph) >> props…` — same create-intermediary shape as node
// (graph& ctor → object_builder<note, …> → new_object<note>). new_object<note> still works.
// Create-seed state lives in create_seed<note> (shared with node / class_def).
class note : public create_seed<note>
{
  public:
    note() { set<OP_printable>(true); }

    // Create-intermediary entry (defined after graph<> is complete in periscope_object_builder.h).
    template<typename underlying_type>
    explicit note(graph<underlying_type>& _graph) noexcept;

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
