#pragma once

#include "misc/periscope_note_properties.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"
#include <cstdio>
#include <cstdlib>
#include <format>
#include <utility>

namespace periscope {
// Forward-declared in periscope_handle.h (default underlying_type = int).
template<typename underlying_type>
class graph;

// note is a class for notes.
// Create spelling: `note& n = note(graph) >> props…` — same create-intermediary shape as node
// (graph& ctor → object_builder<note, …> → new_object<note>). new_object<note> still works.
class note : public object<note>
{
  public:
    note() { set<OP_printable>(true); }

    // Create-intermediary entry (defined after graph<> is complete in periscope_object_builder.h).
    template<typename underlying_type>
    explicit note(graph<underlying_type>& _graph) noexcept;

    note(const note&) = delete;
    note& operator=(const note&) = delete;

    note(note&& other) noexcept
      : object<note>(static_cast<object<note>&&>(other))
      , m_create_graph(std::exchange(other.m_create_graph, nullptr))
      , m_create_fn(std::exchange(other.m_create_fn, nullptr))
    {
    }

    note& operator=(note&& other) noexcept
    {
        if (this != &other) {
            static_cast<object<note>&>(*this) = static_cast<object<note>&&>(other);
            m_create_graph = std::exchange(other.m_create_graph, nullptr);
            m_create_fn = std::exchange(other.m_create_fn, nullptr);
        }
        return *this;
    }

    ~note()
    {
        if (m_create_fn == nullptr)
            return;
        std::fputs("periscope: incomplete note(graph) — discarded without materializing to note& "
                   "(write `note& n = note(g) >> props`, not `note(g) >> props`)\n",
                   stderr);
        std::abort();
    }

    bool is_create_seed() const noexcept { return m_create_fn != nullptr; }

    struct create_seed_state
    {
        void* Graph = nullptr;
        note& (*CreateFn)(void*) = nullptr;
    };

    create_seed_state extract_create() noexcept
    {
        return { std::exchange(m_create_graph, nullptr), std::exchange(m_create_fn, nullptr) };
    }

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

  private:
    void* m_create_graph = nullptr;
    note& (*m_create_fn)(void*) = nullptr;
};
} // namespace periscope
